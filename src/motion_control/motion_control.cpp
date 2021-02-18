#include <Xrender.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include "easy_serial/easy_serial.h"
#include "json/json.h"
#include <motion_control/motion_control.h>
#include "logging/loguru.h"

easy_serial motion_controller("arduino", byte_handler, line_handler);

vector<string> split(std::string str, char delimiter)
{ 
    vector<string> internal; 
    stringstream ss(str); // Turn the string into a stream. 
    string tok; 
    while(getline(ss, tok, delimiter))
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
std::vector<std::string> gcode_stack;
bool torch_on;
unsigned long torch_on_timer;
unsigned long program_run_time;
bool abort_pending;
bool handling_crash;

/*
    Callbacks that get called via okay_callback
*/
void program_finished()
{
    LOG_F(INFO, "M30 Program finished!");
    program_run_time = 0;
    motion_controller_clear_stack();
}
void fire_torch_and_pierce()
{
    okay_callback = &run_pop;
    probe_callback = NULL;
    //Remember that inserting at the top of list means its the next code to run, meaning
    //This list that we are inserting is ran from bottom to top or LIFO mode
    gcode_stack.insert(gcode_stack.begin(), "G90");
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z" + to_string((double)callback_args["cut_height"] - (double)callback_args["pierce_height"]));
    gcode_stack.insert(gcode_stack.begin(), "G4P" + to_string((double)callback_args["pierce_delay"]));
    gcode_stack.insert(gcode_stack.begin(), "M3S1000");
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z" + to_string((double)callback_args["pierce_height"]));
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z0.2"); //Floating head takeup
    torch_on = true;
    torch_on_timer = Xrender_millis();
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
    gcode_stack.insert(gcode_stack.begin(), "G91G0Z0.2"); //Floating head takeup
    torch_on = true;
    torch_on_timer = Xrender_millis();
    run_pop();
}
void torch_off_and_retract()
{
    okay_callback = &run_pop;
    probe_callback = NULL;
    motion_sync_callback = NULL;
    torch_on = false;
    //Remember that inserting at the top of list means its the next code to run, meaning
    //This list that we are inserting is ran from bottom to top or LIFO mode
    gcode_stack.insert(gcode_stack.begin(), "G53 G0 Z0");
    gcode_stack.insert(gcode_stack.begin(), "M5");
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
            std::vector args = split(line, ' ');
            if (args.size() == 3)
            {
                callback_args["pierce_height"] = args[1];
                callback_args["pierce_delay"] = args[2];
                callback_args["cut_height"] = args[3];
                LOG_SCOPE_F(INFO, "Found arguments - %s", callback_args.dump().c_str());
            }
            else
            {
                callback_args["pierce_height"] = 0.150;
                callback_args["pierce_delay"] = 1.2;
                callback_args["cut_height"] = 0.060;
                LOG_SCOPE_F(INFO, "No arguments - Using default!");
            }
            okay_callback = NULL;
            probe_callback = &fire_torch_and_pierce;
            motion_controller_send_crc32("G38.3Z-5F60");
        }
        else if (line.find("touch_torch") != std::string::npos)
        {
            LOG_F(INFO, "[touch_torch] Sending probing cycle! - Waiting for probe to finish!");
            std::vector args = split(line, ' ');
            if (args.size() == 3)
            {
                callback_args["pierce_height"] = args[1];
                callback_args["pierce_delay"] = args[2];
                callback_args["cut_height"] = args[3];
            }
            else
            {
                callback_args["pierce_height"] = 0.150;
                callback_args["pierce_delay"] = 1.2;
                callback_args["cut_height"] = 0.060;
            }
            okay_callback = NULL;
            probe_callback = &touch_torch_and_pierce;
            motion_controller_send_crc32("G38.3Z-5F60");
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
        unsigned long m = (Xrender_millis() - program_run_time);
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
void motion_controller_cmd(std::string cmd)
{
    if (cmd == "abort")
    {
        LOG_F(INFO, "Aborting!");
        motion_controller_send("!");
        abort_pending = true;
        motion_controller_clear_stack();
    }
}
void motion_controller_clear_stack()
{
    gcode_stack.clear();
}
void motion_controller_push_stack(std::string gcode)
{
    gcode.erase(std::remove(gcode.begin(), gcode.end(), ' '), gcode.end());
    gcode_stack.push_back(gcode);
}
void motion_controller_run_stack()
{
    program_run_time = Xrender_millis();
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
void line_handler(std::string line)
{
    //printf("Read line from serial: %s\n", line.c_str());
    if (controller_ready == true)
    {
        if (line.find("{") != std::string::npos)
        {
            try
            {
                dro_data = nlohmann::json::parse(line);
                if (abort_pending == true && (bool)dro_data["IN_MOTION"] == false)
                {
                    motion_controller.delay(300);
                    LOG_F(INFO, "Handling pending abort -> Sending Reset!");
                    motion_controller.send_byte(0x18);
                    motion_controller.delay(300);
                    abort_pending = false;
                    program_run_time = 0;
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
            LOG_F(ERROR, "Checksum failure, aborting program!");
            motion_controller_cmd("abort");
        }
        else if (line.find("error") != std::string::npos)
        {
            LOG_F(ERROR, "Controller Error: %s", line.c_str());
            if (line.find("9") != std::string::npos)
            {
                LOG_F(WARNING, "Program was aborted because floating head or ohmic touch input was activated before probing cycle began!");
            }
        }
        else if (line.find("[CRASH]") != std::string::npos && handling_crash == false)
        {
            if (torch_on == true && (Xrender_millis() - torch_on_timer) > 2000)
            {
                LOG_F(INFO, "Program was aborted because troch crash was detected!");
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
        }
    }
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
            if (motion_sync_callback != NULL && (bool)dro_data["IN_MOTION"] == false && (Xrender_millis() - program_run_time) > 500)
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
void motion_control_init()
{
    controller_ready = false;
    torch_on = false;
    torch_on_timer = 0;
    abort_pending = false;
    handling_crash = false;
    program_run_time = 0;
    Xrender_push_timer(100, motion_control_status_timer);

    
}
void motion_control_tick()
{
    motion_controller.tick();
    if (motion_controller.is_connected == false)
    {
        controller_ready = false;
    }
}