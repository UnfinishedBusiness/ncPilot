
#include "motion_control.h"
#include <dialogs/dialogs.h>

easy_serial motion_controller("arduino", byte_handler, line_handler);

void removeSubstrs(std::string& s, std::string p) { 
  std::string::size_type n = p.length();
  for (std::string::size_type i = s.find(p);
      i != std::string::npos;
      i = s.find(p))
      s.erase(i, n);
}
std::vector<std::string> split(std::string str, char delimiter)
{ 
    std::vector<std::string> internal; 
    std::stringstream ss(str); // Turn the string into a stream. 
    std::string tok; 
    while(std::getline(ss, tok, delimiter))
    { 
        internal.push_back(tok); 
    } 
    return internal; 
}
nlohmann::json dro_data;
nlohmann::json callback_args;
bool controller_ready;
void (*okay_callback)() = NULL;
void (*probe_callback)() = NULL;
void (*motion_sync_callback)() = NULL;
void (*arc_okay_callback)() = NULL;
int arc_retry_count = 0;
std::vector<std::string> gcode_stack;
bool torch_on;
unsigned long torch_on_timer;
unsigned long arc_okay_timer;
unsigned long program_run_time;
bool abort_pending;
bool handling_crash;
bool send_parameters = true;

/*
    Callbacks that get called via okay_callback
*/
void program_finished()
{
    LOG_F(INFO, "M30 Program finished!");
    motion_controller_log_runtime();
    program_run_time = 0;
    arc_retry_count = 0;
    okay_callback = NULL;
    probe_callback = NULL;
    motion_sync_callback = NULL;
    arc_okay_callback = NULL;
    motion_controller_clear_stack();
}
bool arc_okay_expire_timer()
{
    if (abort_pending == true) return false; //Pending abort must cancel this timer otherwise aborts would be ignored in arc retry loop!
    /*
        if this function is called, torch fired but controller never seen an arc okay signal after 3 seconds...
    */
    if (arc_okay_callback != NULL) //Only run this code if the arc_okay_callback was not processed, meaning the timr event fired and the arc never transfered after n seconds
    {
        okay_callback = &run_pop;
        probe_callback = NULL;
        motion_sync_callback = NULL;
        arc_okay_callback = NULL;
        torch_on = false;
        //Remember that inserting at the top of list means its the next code to run, meaning
        //This list that we are inserting is ran from bottom to top or LIFO mode
        gcode_stack.insert(gcode_stack.begin(), "fire_torch " + std::to_string((double)callback_args["pierce_height"]) + " " + std::to_string((double)callback_args["pierce_delay"]) + " " + std::to_string((double)callback_args["cut_height"]));
        gcode_stack.insert(gcode_stack.begin(), "G53 G0 Z0"); //Retract
        gcode_stack.insert(gcode_stack.begin(), "G90"); //Absolute mode
        gcode_stack.insert(gcode_stack.begin(), "M5"); //Torch Off
        LOG_F(INFO, "Running callback => arc_okay_expire_timer() - retry count: %d", arc_retry_count);
        run_pop();
        arc_retry_count++;
    }
    return false; //Don't repeat
}
void raise_to_cut_height_and_run_program()
{
    okay_callback = &run_pop;
    probe_callback = NULL;
    arc_okay_callback = NULL;
    arc_retry_count = 0;
    //Remember that inserting at the top of list means its the next code to run, meaning
    //This list that we are inserting is ran from bottom to top or LIFO mode
    gcode_stack.insert(gcode_stack.begin(), "G90");
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z" + std::to_string((double)callback_args["cut_height"] - (double)callback_args["pierce_height"]));
    gcode_stack.insert(gcode_stack.begin(), "G4P" + std::to_string((double)callback_args["pierce_delay"]));
    LOG_F(INFO, "Running callback => raise_to_cut_height_and_run_program()");
    run_pop();
}
void raise_to_pierce_height_and_fire_torch()
{
    okay_callback = &run_pop;
    probe_callback = NULL;
    //Remember that inserting at the top of list means its the next code to run, meaning
    //This list that we are inserting is ran from bottom to top or LIFO mode
    gcode_stack.insert(gcode_stack.begin(), "WAIT_FOR_ARC_OKAY");
    gcode_stack.insert(gcode_stack.begin(), "M3S1000");
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z" + std::to_string((double)callback_args["pierce_height"]));
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z" + std::to_string(globals->machine_parameters.floating_head_backlash));
    LOG_F(INFO, "Running callback => raise_to_pierce_height_and_fire_torch()");
    torch_on = true;
    torch_on_timer = EasyRender::Millis();
    run_pop();
}
void touch_torch_and_pierce()
{
    okay_callback = &run_pop;
    probe_callback = NULL;
    //Remember that inserting at the top of list means its the next code to run, meaning
    //This list that we are inserting is ran from bottom to top or LIFO mode
    gcode_stack.insert(gcode_stack.begin(), "G90");
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z0.5");
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z" + std::to_string(globals->machine_parameters.floating_head_backlash));
    LOG_F(INFO, "Touching off torch and dry running!");
    run_pop();
}
void torch_off_and_abort()
{
    okay_callback = &run_pop;
    probe_callback = NULL;
    motion_sync_callback = NULL;
    arc_okay_callback = NULL;
    torch_on = false;
    //Remember that inserting at the top of list means its the next code to run, meaning
    //This list that we are inserting is ran from bottom to top or LIFO mode
    gcode_stack.insert(gcode_stack.begin(), "M30");
    gcode_stack.insert(gcode_stack.begin(), "G53 G0 Z0");
    gcode_stack.insert(gcode_stack.begin(), "M5");
    LOG_F(INFO, "Shutting torch off, retracting, and aborting!");
    run_pop();
}
void torch_off_and_retract()
{
    okay_callback = &run_pop;
    probe_callback = NULL;
    motion_sync_callback = NULL;
    arc_okay_callback = NULL;
    torch_on = false;
    //Remember that inserting at the top of list means its the next code to run, meaning
    //This list that we are inserting is ran from bottom to top or LIFO mode
    gcode_stack.insert(gcode_stack.begin(), "G53 G0 Z0");
    gcode_stack.insert(gcode_stack.begin(), "M5");
    LOG_F(INFO, "Shutting torch off and retracting!");
    run_pop();
}
void run_pop()
{
    if (gcode_stack.size() > 0)
    {
        std::string line = gcode_stack[0];
        gcode_stack.erase(gcode_stack.begin());
        if (line.find("fire_torch") != std::string::npos)
        {
            LOG_F(INFO, "[fire_torch] Sending probing cycle! - Waiting for probe to finish!");
            std::vector<std::string> args = split(line, ' ');
            if (args.size() == 4)
            {
                callback_args["pierce_height"] = (double)atof(args[1].c_str());
                callback_args["pierce_delay"] = (double)atof(args[2].c_str());
                callback_args["cut_height"] = (double)atof(args[3].c_str());
                LOG_F(INFO, "Found arguments - %s", callback_args.dump().c_str());
            }
            else
            {
                callback_args["pierce_height"] = 0.150f;
                callback_args["pierce_delay"] = 1.2f;
                callback_args["cut_height"] = 0.060f;
                LOG_F(INFO, "No arguments - Using default!");
            }
            if (arc_retry_count > 3)
            {
                LOG_F(INFO, "[run_pop] Arc retry max count reached. Retracting and aborting program!");
                okay_callback = NULL;
                motion_sync_callback = &torch_off_and_abort;
                dialogs_set_info_value("Arc Strike Retry max count expired!\nLikely causes are:\n1. Bad or worn out consumables.\n2. Faulty work clamp connection or wire\n3. Inadequate pressure and/or moisture in the air system\n4. Dirty material and/or non-conductive surface\n5. Faulty Cutting unit, eg. Plasma Power Unit is worn out or broken");
            }
            else
            {
                okay_callback = NULL;
                probe_callback = &raise_to_pierce_height_and_fire_torch;
                motion_controller_send_crc32("G38.3Z" + std::to_string(globals->machine_parameters.machine_extents[2]) + "F" + std::to_string(globals->machine_parameters.z_probe_feedrate));
            }
        }
        else if (line.find("touch_torch") != std::string::npos)
        {
            LOG_F(INFO, "[touch_torch] Sending probing cycle! - Waiting for probe to finish!");
            std::vector<std::string> args = split(line, ' ');
            if (args.size() == 4)
            {
                callback_args["pierce_height"] = (double)atof(args[1].c_str());
                callback_args["pierce_delay"] = (double)atof(args[2].c_str());
                callback_args["cut_height"] = (double)atof(args[3].c_str());
            }
            else
            {
                callback_args["pierce_height"] = 0.150f;
                callback_args["pierce_delay"] = 1.2f;
                callback_args["cut_height"] = 0.060f;
            }
            okay_callback = NULL;
            probe_callback = &touch_torch_and_pierce;
            motion_controller_send_crc32("G38.3Z" + std::to_string(globals->machine_parameters.machine_extents[2]) + "F" + std::to_string(globals->machine_parameters.z_probe_feedrate));
        }
        else if(line.find("WAIT_FOR_ARC_OKAY") != std::string::npos)
        {
            LOG_F(INFO, "[run_pop] Setting arc_okay callback and arc_okay expire timer => fires in %.4f ms!", (3 + (double)callback_args["pierce_delay"]) * 1000);
            globals->renderer->PushTimer((3 + (double)callback_args["pierce_delay"]) * 1000, &arc_okay_expire_timer);
            arc_okay_callback = &raise_to_cut_height_and_run_program;
            okay_callback = NULL;
            probe_callback = NULL;
        }
        else if (line.find("torch_off") != std::string::npos)
        {
            okay_callback = NULL;
            motion_sync_callback = &torch_off_and_retract;
        }
        else if (line.find("M30") != std::string::npos)
        {
            motion_sync_callback = &program_finished;
            okay_callback = NULL;
            probe_callback = NULL;
        }
        else
        {
            line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
            LOG_F(INFO, "(runpop) sending %s", line.c_str());
            motion_controller_send_crc32(line);
        }
    }
    else
    {
        okay_callback = NULL;
    }
    
}
/*          end callbacks           */
nlohmann::json motion_controller_get_dro()
{
    return dro_data;
}
nlohmann::json motion_controller_get_run_time()
{
    if (program_run_time > 0)
    {
        unsigned long m = (EasyRender::Millis() - program_run_time);
        unsigned long seconds=(m/1000)%60;
        unsigned long minutes=(m/(1000*60))%60;
        unsigned long hours=(m/(1000*60*60))%24;
        return {
            {"seconds", seconds},
            {"minutes", minutes},
            {"hours", hours}
        };
    }
    else
    {
        return NULL;
    }
}
void motion_controller_log_runtime()
{
    unsigned long m = (EasyRender::Millis() - program_run_time);
    unsigned long seconds=(m/1000)%60;
    unsigned long minutes=(m/(1000*60))%60;
    unsigned long hours=(m/(1000*60*60))%24;
    LOG_F(INFO, "Logging program run time %luh %lum %lus to total", hours, minutes, seconds);
    std::ifstream runtime_file(globals->renderer->GetConfigDirectory() + "runtime.json");
    if (runtime_file.is_open())
    {
        std::string uptime_json_string((std::istreambuf_iterator<char>(runtime_file)), std::istreambuf_iterator<char>());
        nlohmann::json runtime_json = nlohmann::json::parse(uptime_json_string.c_str());
        try
        {
            hours += (unsigned long)runtime_json["hours"];
            minutes += (unsigned long)runtime_json["minutes"];
            seconds += (unsigned long)runtime_json["seconds"];
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing uptime file!");
        }
    }
    runtime_file.close();
    nlohmann::json runtime;
    runtime["hours"] = hours;
    runtime["minutes"] = minutes;
    runtime["seconds"] = seconds;
    std::ofstream out(globals->renderer->GetConfigDirectory() + "runtime.json");
    out << runtime.dump();
    out.close();
}
bool motion_controller_is_torch_on()
{
    return torch_on;
}
void motion_controller_cmd(std::string cmd)
{
    if (cmd == "abort")
    {
        LOG_F(INFO, "Aborting!");
        motion_controller_send("!");
        abort_pending = true;
        okay_callback = NULL;
        probe_callback = NULL;
        motion_sync_callback = NULL;
        arc_okay_callback = NULL;
        motion_controller_clear_stack();
    }
}
void motion_controller_clear_stack()
{
    gcode_stack.clear();
}
void motion_controller_push_stack(std::string gcode)
{
    gcode_stack.push_back(gcode);
}
void motion_controller_run_stack()
{
    program_run_time = EasyRender::Millis();
    run_pop();
    okay_callback = &run_pop;
}

uint32_t motion_controller_crc32c(uint32_t crc, const char *buf, size_t len)
{
    int k;
    crc = ~crc;
    while (len--) {
        crc ^= *buf++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return ~crc;
}
void motion_controller_log_controller_error(int error)
{
    std::string ret;
    switch(error)
    {
        case 1: ret = "G-code words consist of a letter and a value. Letter was not found"; break;
        case 2: ret = "Numeric value format is not valid or missing an expected value."; break;
        case 3: ret = "System command was not recognized or supported."; break;
        case 4: ret = "Negative value received for an expected positive value."; break;
        case 5: ret = "Homing cycle is not enabled via settings."; break;
        case 6: ret = "Minimum step pulse time must be greater than 3usec"; break;
        case 7: ret = "EEPROM read failed. Reset and restored to default values."; break;
        case 8: ret = "Real-Time command cannot be used unless machine is IDLE. Ensures smooth operation during a job."; break;
        case 9: ret = "G-code locked out during alarm or jog state"; break;
        case 10: ret = "Soft limits cannot be enabled without homing also enabled."; break;
        case 11: ret = "Max characters per line exceeded. Line was not processed and executed."; break;
        case 12: ret = "Setting value exceeds the maximum step rate supported."; break;
        case 13: ret = "Safety door detected as opened and door state initiated."; break;
        case 14: ret = "Build info or startup line exceeded EEPROM line length limit."; break;
        case 15: ret = "Jog target exceeds machine travel. Command ignored."; break;
        case 16: ret = "Jog command with no '=' or contains prohibited g-code."; break;
        case 17: ret = "Laser mode requires PWM output."; break;
        case 20: ret = "Unsupported or invalid g-code command found in block."; break;
        case 21: ret = "More than one g-code command from same modal group found in block."; break;
        case 22: ret = "Feed rate has not yet been set or is undefined."; break;
        case 23: ret = "G-code command in block requires an integer value."; break;
        case 24: ret = "Two G-code commands that both require the use of the XYZ axis words were detected in the block."; break;
        case 25: ret = "A G-code word was repeated in the block."; break;
        case 26: ret = "A G-code command implicitly or explicitly requires XYZ axis words in the block, but none were detected."; break;
        case 27: ret = "N line number value is not within the valid range of 1 - 9,999,999."; break;
        case 28: ret = "A G-code command was sent, but is missing some required P or L value words in the line."; break;
        case 29: ret = "System only supports six work coordinate systems G54-G59. G59.1, G59.2, and G59.3 are not supported."; break;
        case 30: ret = "The G53 G-code command requires either a G0 seek or G1 feed motion mode to be active. A different motion was active."; break;
        case 31: ret = "There are unused axis words in the block and G80 motion mode cancel is active."; break;
        case 32: ret = "A G2 or G3 arc was commanded but there are no XYZ axis words in the selected plane to trace the arc."; break;
        case 33: ret = "The motion command has an invalid target. G2, G3, and G38.2 generates this error, if the arc is impossible to generate or if the probe target is the current position."; break;
        case 34: ret = "A G2 or G3 arc, traced with the radius definition, had a mathematical error when computing the arc geometry. Try either breaking up the arc into semi-circles or quadrants, or redefine them with the arc offset definition."; break;
        case 35: ret = "A G2 or G3 arc, traced with the offset definition, is missing the IJK offset word in the selected plane to trace the arc."; break;
        case 36: ret = "There are unused, leftover G-code words that aren't used by any command in the block."; break;
        case 37: ret = "The G43.1 dynamic tool length offset command cannot apply an offset to an axis other than its configured axis. The Grbl default axis is the Z-axis."; break;
        case 38: ret = "Tool number greater than max supported value."; break;
        case 100: ret = "Communication Redundancy Check Failed. Program Aborted to ensure unintended behavior does not occur!"; break;
        default: ret = "unknown"; break;
    }
    LOG_F(ERROR, "Firmware Error %d => %s", error, ret.c_str());
}
void line_handler(std::string line)
{
    if (controller_ready == true)
    {
        if (line.find("{") != std::string::npos)
        {
            try
            {
                dro_data = nlohmann::json::parse(line);
                if (dro_data["ARC_OK"] == false) //Logic is inverted. Bool goes low when controller has arc_ok signal
                {
                    if (arc_okay_callback != NULL)
                    {
                        arc_okay_callback();
                        arc_okay_callback = NULL;
                        arc_okay_timer = EasyRender::Millis();
                    }
                }
                if (abort_pending == true && (bool)dro_data["IN_MOTION"] == false)
                {
                    motion_controller_log_runtime();
                    motion_controller.delay(300);
                    LOG_F(INFO, "Handling pending abort -> Sending Reset!");
                    motion_controller.send_byte(0x18);
                    motion_controller.delay(300);
                    abort_pending = false;
                    program_run_time = 0;
                    arc_retry_count = 0;
                    torch_on = false;
                    handling_crash = false;
                    controller_ready = false;
                }
            }
            catch(...)
            {
                LOG_F(ERROR, "Error parsing DRO JSON data!");
            }
        }
        else if (line.find("[CHECKSUM_FAILURE]") != std::string::npos)
        {
            dialogs_set_info_value("Communication Checksum failure, aborting program!");
            motion_controller_cmd("abort");
        }
        else if (line.find("error") != std::string::npos)
        {
            if (line.find("9") != std::string::npos)
            {
                dialogs_set_info_value("Program was aborted because floating head or ohmic touch input was activated before probing cycle began!");
            }
            removeSubstrs(line, "error:");
            motion_controller_log_controller_error(atoi(line.c_str()));
        }
        else if (line.find("[CRASH]") != std::string::npos && handling_crash == false)
        {
            if (torch_on == true && (EasyRender::Millis() - torch_on_timer) > 2000)
            {
                dialogs_set_info_value("Program was aborted because torch crash was detected!");
                motion_controller_cmd("abort");
                handling_crash = true;
            }
        }
        else if (line.find("[PRB") != std::string::npos)
        {
            LOG_F(INFO, "Probe finished - Running callback!");
            if (probe_callback != NULL) probe_callback();
        }
        else
        {
            LOG_F(WARNING, "Unidentified line recived - %s", line.c_str());
        }
    }
    if (controller_ready == false)
    {
        if (line.find("Grbl") != std::string::npos)
        {
            LOG_F(INFO, "Controller ready!");
            controller_ready = true;

            if (send_parameters == true)
            {
                send_parameters = false;
                uint8_t dir_invert_mask = 0b00000000;
                if (globals->machine_parameters.axis_invert[0]) dir_invert_mask |= (1 << 0);
                if (globals->machine_parameters.axis_invert[1]) dir_invert_mask |= (1 << 1);
                if (globals->machine_parameters.axis_invert[2]) dir_invert_mask |= (1 << 2);
                if (globals->machine_parameters.axis_invert[3]) dir_invert_mask |= (1 << 3);

                motion_controller_push_stack("$0=" + std::to_string(50));
                motion_controller_push_stack("$3=" + std::to_string(dir_invert_mask));
                motion_controller_push_stack("$11=" + std::to_string(globals->machine_parameters.junction_deviation));
                motion_controller_push_stack("$100=" + std::to_string(globals->machine_parameters.axis_scale[0]));
                motion_controller_push_stack("$101=" + std::to_string(globals->machine_parameters.axis_scale[1]));
                motion_controller_push_stack("$102=" + std::to_string(globals->machine_parameters.axis_scale[2]));
                motion_controller_push_stack("$110=" + std::to_string(globals->machine_parameters.max_vel[0]));
                motion_controller_push_stack("$111=" + std::to_string(globals->machine_parameters.max_vel[1]));
                motion_controller_push_stack("$112=" + std::to_string(globals->machine_parameters.max_vel[2]));
                motion_controller_push_stack("$120=" + std::to_string(globals->machine_parameters.max_accel[0]));
                motion_controller_push_stack("$121=" + std::to_string(globals->machine_parameters.max_accel[1]));
                motion_controller_push_stack("$122=" + std::to_string(globals->machine_parameters.max_accel[2]));
                motion_controller_push_stack("G10 L2 P0 X" + std::to_string(globals->machine_parameters.work_offset[0]) + " Y" + std::to_string(globals->machine_parameters.work_offset[1]) + " Z0");
                motion_controller_push_stack("M30");
                motion_controller_run_stack();
            }
        }
    }
}
void motion_controller_save_machine_parameters()
{
    nlohmann::json preferences;
    preferences["work_offset"]["x"] = globals->machine_parameters.work_offset[0];
    preferences["work_offset"]["y"] = globals->machine_parameters.work_offset[1];
    preferences["work_offset"]["z"] = globals->machine_parameters.work_offset[2];
    preferences["machine_extents"]["x"] = globals->machine_parameters.machine_extents[0];
    preferences["machine_extents"]["y"] = globals->machine_parameters.machine_extents[1];
    preferences["machine_extents"]["z"] = globals->machine_parameters.machine_extents[2];
    preferences["cutting_extents"]["x1"] = globals->machine_parameters.cutting_extents[0];
    preferences["cutting_extents"]["y1"] = globals->machine_parameters.cutting_extents[1];
    preferences["cutting_extents"]["x2"] = globals->machine_parameters.cutting_extents[2];
    preferences["cutting_extents"]["y2"] = globals->machine_parameters.cutting_extents[3];
    preferences["axis_scale"]["x"] = globals->machine_parameters.axis_scale[0];
    preferences["axis_scale"]["y"] = globals->machine_parameters.axis_scale[1];
    preferences["axis_scale"]["z"] = globals->machine_parameters.axis_scale[2];
    preferences["max_vel"]["x"] = globals->machine_parameters.max_vel[0];
    preferences["max_vel"]["y"] = globals->machine_parameters.max_vel[1];
    preferences["max_vel"]["z"] = globals->machine_parameters.max_vel[2];
    preferences["max_accel"]["x"] = globals->machine_parameters.max_accel[0];
    preferences["max_accel"]["y"] = globals->machine_parameters.max_accel[1];
    preferences["max_accel"]["z"] = globals->machine_parameters.max_accel[2];
    preferences["junction_deviation"] = globals->machine_parameters.junction_deviation;
    preferences["arc_stablization_time"] = globals->machine_parameters.arc_stablization_time;
    preferences["floating_head_backlash"] = globals->machine_parameters.floating_head_backlash;
    preferences["z_probe_feedrate"] = globals->machine_parameters.z_probe_feedrate;
    preferences["axis_invert"]["x"] = globals->machine_parameters.axis_invert[0];
    preferences["axis_invert"]["y1"] = globals->machine_parameters.axis_invert[1];
    preferences["axis_invert"]["y2"] = globals->machine_parameters.axis_invert[2];
    preferences["axis_invert"]["z"] = globals->machine_parameters.axis_invert[3];
    globals->machine_plane->bottom_left.x = 0;
    globals->machine_plane->bottom_left.y = 0;
    globals->machine_plane->width = globals->machine_parameters.machine_extents[0];
    globals->machine_plane->height = globals->machine_parameters.machine_extents[1];
    globals->cuttable_plane->bottom_left.x = globals->machine_parameters.cutting_extents[0];
    globals->cuttable_plane->bottom_left.y = globals->machine_parameters.cutting_extents[1];
    globals->cuttable_plane->width = (globals->machine_parameters.machine_extents[0] + globals->machine_parameters.cutting_extents[2]) - globals->machine_parameters.cutting_extents[0];
    globals->cuttable_plane->height = (globals->machine_parameters.machine_extents[1] + globals->machine_parameters.cutting_extents[3]) - globals->machine_parameters.cutting_extents[1];
    try
    {
        std::ofstream out(globals->renderer->GetConfigDirectory() + "machine_parameters.json");
        out << preferences.dump();
        out.close();
    }
    catch(...)
    {
        LOG_F(ERROR, "Could not write parameters file!");
    }
}
void motion_controller_trigger_reset()
{
    LOG_F(INFO, "Resetting Motion Controller!");
    motion_controller.serial.setDTR(true);
    motion_controller.delay(100);
    motion_controller.serial.setDTR(false);
    controller_ready = false;
    send_parameters = true;
}
bool byte_handler(uint8_t b)
{
    if (controller_ready == true)
    {
        if (b == '>')
        {
            LOG_F(INFO, "Recieved ok byte!");
            if (okay_callback != NULL) okay_callback();
            return true;
        }
    }
    return false;
}
void motion_controller_send_crc32(std::string s)
{
    if (controller_ready == true)
    {
        s.erase(remove(s.begin(), s.end(), ' '), s.end());
        s.erase(remove(s.begin(), s.end(), '\n'), s.end());
        s.erase(remove(s.begin(), s.end(), '\r'), s.end());
        uint32_t checksum = motion_controller_crc32c(0, s.c_str(), s.size());
        motion_controller.send_string(s + "*" + std::to_string(checksum) + "\n");
    }
}
void motion_controller_send(std::string s)
{
    if (controller_ready == true)
    {
        s.erase(remove(s.begin(), s.end(), ' '), s.end());
        s.erase(remove(s.begin(), s.end(), '\n'), s.end());
        s.erase(remove(s.begin(), s.end(), '\r'), s.end());
        motion_controller.send_string(s + "\n");
    }
}
bool motion_control_status_timer()
{
    //also use this to check if auto thc should be turned on...
    if (controller_ready == true)
    {
        if (dro_data.contains("STATUS"))
        {
            if (motion_sync_callback != NULL && (bool)dro_data["IN_MOTION"] == false && (EasyRender::Millis() - program_run_time) > 500)
            {
                LOG_F(INFO, "Motion is synced, calling pending callback!");
                motion_sync_callback();
                motion_sync_callback = NULL;
            }
        }
    }
    motion_controller_send("?");
    return true;
}
bool motion_control_update_firmware_hide_info_window()
{
    dialogs_show_info_window(false);
    return false;
}
bool motion_control_update_firmware()
{
    /*
        First we need to download the firmware...
    */
    motion_controller.serial.close();
    controller_ready = false;
    stk500_write_program(std::string(globals->renderer->GetConfigDirectory() + "firmware.hex").c_str(), motion_controller.serial_port.c_str());
    globals->renderer->PushTimer(1000, &motion_control_update_firmware_hide_info_window);
    return false;
}
void motion_control_init()
{
    controller_ready = false;
    torch_on = false;
    torch_on_timer = 0;
    arc_okay_timer = 0;
    abort_pending = false;
    handling_crash = false;
    program_run_time = 0;
    arc_retry_count = 0;
    globals->renderer->PushTimer(100, motion_control_status_timer);
}
void motion_control_tick()
{
    motion_controller.tick();
    if (motion_controller.is_connected == false)
    {
        controller_ready = false;
    }
    try
    {
        if (torch_on == true && (EasyRender::Millis() - arc_okay_timer) > (globals->machine_parameters.arc_stablization_time + ((float)callback_args["pierce_delay"] * 1000)))
        {
            //LOG_F(INFO, "Arc is stabalized!");
            /*
                Need to implement real-time command that takes 4 bytes.
                byte 1 -> THC set command
                byte 2 -> value low bit
                byte 3 -> value high bit
                byte 4 -> End THC set command

                This way THC can be changed on the fly withouth injecting into the gcode stream which under long line moves could b
                some time after this event occurs... Also would allow setting thc on the fly manually
            */
        }
    }
    catch(...)
    {
        LOG_F(ERROR, "Error parsing callback args for Smart THC");
    }
}