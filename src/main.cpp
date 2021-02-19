#include "Xrender.h"
#include "application.h"
#include "json/json.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "debug/debug.h"
#include "hmi/hmi.h"
#include "logging/loguru.h"
#include "event_handling/event_handling.h"
#include "menu_bar/menu_bar.h"
#include "dialogs/dialogs.h"
#include "utility/utility.h"
#include "motion_control/motion_control.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "net_skeleton/net_skeleton.h"

// This event handler implements TCP echo server
static void ev_handler(struct ns_connection *nc, int ev, void *ev_data)
{
    struct iobuf *io = &nc->recv_iobuf;
    if (ev == NS_RECV)
    {
        char ip[100];
        int flags = 0;
        flags |= 1UL << 2;
        flags |= 1UL << 4;
        ns_sock_to_str(nc->sock, ip, 100, flags);
        LOG_F(INFO, "Recieved: %s from %s", io->buf, ip);
        ns_send(nc, io->buf, io->len);  // Echo received data back
        iobuf_remove(io, io->len);      // Discard data from recv buffer
    }
}

global_variables_t *globals;

void init_preferences()
{
    std::ifstream preferences_file(Xrender_get_config_dir("ncPilot") + "preferences.json");
    if (preferences_file.is_open())
    {
        std::string preference_json_string((std::istreambuf_iterator<char>(preferences_file)), std::istreambuf_iterator<char>());
        nlohmann::json preferences = nlohmann::json::parse(preference_json_string.c_str());
        try
        {
            globals->preferences.background_color[0] = (double)preferences["background_color"]["r"] / 255;
            globals->preferences.background_color[1] = (double)preferences["background_color"]["g"] / 255;
            globals->preferences.background_color[2] = (double)preferences["background_color"]["b"] / 255;
            globals->preferences.machine_plane_color[0] = (double)preferences["machine_plane_color"]["r"] / 255;
            globals->preferences.machine_plane_color[1] = (double)preferences["machine_plane_color"]["g"] / 255;
            globals->preferences.machine_plane_color[2] = (double)preferences["machine_plane_color"]["b"] / 255;
            globals->preferences.cuttable_plane_color[0] = (double)preferences["cuttable_plane_color"]["r"] / 255;
            globals->preferences.cuttable_plane_color[1] = (double)preferences["cuttable_plane_color"]["g"] / 255;
            globals->preferences.cuttable_plane_color[2] = (double)preferences["cuttable_plane_color"]["b"] / 255;
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing preferences file!");
        }
    }
    else
    {
        LOG_F(WARNING, "Preferences file does not exist, creating it!");
        globals->preferences.background_color[0] = 8.0f / 255;
        globals->preferences.background_color[1] = 14.0f / 255;
        globals->preferences.background_color[2] = 84.0f / 255;
        globals->preferences.machine_plane_color[0] = 100.0f / 255;
        globals->preferences.machine_plane_color[1] = 100.0f / 255;
        globals->preferences.machine_plane_color[2] = 100.0f / 255;
        globals->preferences.cuttable_plane_color[0] = 151.0f / 255;
        globals->preferences.cuttable_plane_color[1] = 5.0f / 255;
        globals->preferences.cuttable_plane_color[2] = 5.0f / 255;
    }

    std::ifstream json_file(Xrender_get_config_dir("ncPilot") + "machine_parameters.json");
    if (json_file.is_open())
    {
        std::string json_string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
        nlohmann::json parameters = nlohmann::json::parse(json_string.c_str());
        try
        {
            LOG_F(INFO, "Found %s!", string(Xrender_get_config_dir("ncPilot") + "machine_parameters.json").c_str());
            globals->machine_parameters.work_offset[0] = (float)parameters["work_offset"]["x"];
            globals->machine_parameters.work_offset[1] = (float)parameters["work_offset"]["y"];
            globals->machine_parameters.work_offset[2] = (float)parameters["work_offset"]["z"];
            globals->machine_parameters.machine_extents[0] = (float)parameters["machine_extents"]["x"];
            globals->machine_parameters.machine_extents[1] = (float)parameters["machine_extents"]["y"];
            globals->machine_parameters.machine_extents[2] = (float)parameters["machine_extents"]["z"];
            globals->machine_parameters.cutting_extents[0] = (float)parameters["cutting_extents"]["x1"];
            globals->machine_parameters.cutting_extents[1] = (float)parameters["cutting_extents"]["y1"];
            globals->machine_parameters.cutting_extents[2] = (float)parameters["cutting_extents"]["x2"];
            globals->machine_parameters.cutting_extents[3] = (float)parameters["cutting_extents"]["y2"];
            globals->machine_parameters.axis_scale[0] = (float)parameters["axis_scale"]["x"];
            globals->machine_parameters.axis_scale[1] = (float)parameters["axis_scale"]["y"];
            globals->machine_parameters.axis_scale[2] = (float)parameters["axis_scale"]["z"];
            globals->machine_parameters.max_vel[0] = (float)parameters["max_vel"]["x"];
            globals->machine_parameters.max_vel[1] = (float)parameters["max_vel"]["y"];
            globals->machine_parameters.max_vel[2] = (float)parameters["max_vel"]["z"];
            globals->machine_parameters.max_accel[0] = (float)parameters["max_accel"]["x"];
            globals->machine_parameters.max_accel[1] = (float)parameters["max_accel"]["y"];
            globals->machine_parameters.max_accel[2] = (float)parameters["max_accel"]["z"];
            globals->machine_parameters.junction_deviation = (float)parameters["junction_deviation"];
            globals->machine_parameters.floating_head_backlash = (float)parameters["floating_head_backlash"];
            globals->machine_parameters.z_probe_feedrate = (float)parameters["z_probe_feedrate"];
            globals->machine_parameters.axis_invert[0] = (bool)parameters["axis_invert"]["x"];
            globals->machine_parameters.axis_invert[1] = (bool)parameters["axis_invert"]["y1"];
            globals->machine_parameters.axis_invert[2] = (bool)parameters["axis_invert"]["y2"];
            globals->machine_parameters.axis_invert[3] = (bool)parameters["axis_invert"]["z"];
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing Machine Parameters file!");
        }
    }
    else
    {
        LOG_F(WARNING, "%s does not exist, using default parameters!", string(Xrender_get_config_dir("ncPilot") + "machine_parameters.json").c_str());
        globals->machine_parameters.work_offset[0] = 0.0f;
        globals->machine_parameters.work_offset[1] = 0.0f;
        globals->machine_parameters.work_offset[2] = 0.0f;
        globals->machine_parameters.machine_extents[0] = 48.0f;
        globals->machine_parameters.machine_extents[1] = 96.0f;
        globals->machine_parameters.machine_extents[2] = -6.0f;
        globals->machine_parameters.cutting_extents[0] = 1.0f;
        globals->machine_parameters.cutting_extents[1] = 1.0f;
        globals->machine_parameters.cutting_extents[2] = -1.0f;
        globals->machine_parameters.cutting_extents[3] = -1.0f;
        globals->machine_parameters.axis_scale[0] = 1016.002f;
        globals->machine_parameters.axis_scale[1] = 1016.002f;
        globals->machine_parameters.axis_scale[2] = 1016.002f;
        globals->machine_parameters.max_vel[0] = 230.0f;
        globals->machine_parameters.max_vel[1] = 400.0f;
        globals->machine_parameters.max_vel[2] = 200.0f;
        globals->machine_parameters.max_accel[0] = 8.0f;
        globals->machine_parameters.max_accel[1] = 8.0f;
        globals->machine_parameters.max_accel[2] = 20.0f;
        globals->machine_parameters.junction_deviation = 0.0005f;
        globals->machine_parameters.floating_head_backlash = 0.200f;
        globals->machine_parameters.z_probe_feedrate = 40.0f;
        globals->machine_parameters.axis_invert[0] = true;
        globals->machine_parameters.axis_invert[1] = true;
        globals->machine_parameters.axis_invert[2] = true;
        globals->machine_parameters.axis_invert[3] = true;
    }
}
int main(int argc, char **argv)
{
    loguru::init(argc, argv);
    if (!utility_dir_exists(Xrender_get_config_dir("ncPilot").c_str()))
    {
        #if defined(_WIN32)
            _mkdir(Xrender_get_config_dir("ncPilot").c_str()); // can be used on Windows
        #else 
            mkdir(Xrender_get_config_dir("ncPilot").c_str(), 0700);
        #endif
    }
    struct ns_mgr mgr;
    ns_mgr_init(&mgr, NULL);  // 2
    ns_bind(&mgr, "1414", ev_handler);  // 3
    /*
        Initialize global variables
    */
    globals = new global_variables_t;
    globals->zoom = 5;
    globals->pan.x = 0;
    globals->pan.y = 0;
    globals->mouse_pos_in_screen_coordinates = {0, 0};
    globals->mouse_pos_in_matrix_coordinates = {0, 0};

    init_preferences();
    loguru::add_file(string(Xrender_get_config_dir("ncPilot") + "ncPilot.log").c_str(), loguru::Append, loguru::Verbosity_MAX);
    // Only show most relevant things on stderr:
    loguru::g_stderr_verbosity = 1;
    
    LOG_F(INFO, "Config directory: %s", Xrender_get_config_dir("ncPilot").c_str());

    if (Xrender_init({
        {"window_title", "ncPilot"},
        {"ini_file_name", Xrender_get_config_dir("ncPilot") + "gui.ini"}, 
        {"log_file_name", Xrender_get_config_dir("ncPilot") + "ncPilot.log"}, 
        {"maximize", false}, 
        {"window_width", 1200}, 
        {"window_height", 700}, 
        {"clear_color", { 
                {"r", (double)globals->preferences.background_color[0] * 255}, 
                {"g", (double)globals->preferences.background_color[1] * 255}, 
                {"b", (double)globals->preferences.background_color[2] * 255}, 
                {"a", 255}
        }}
    }))
    {
        globals->Xcore = Xrender_get_core_variables();
        debug_init();
        event_handling_init();
        dialogs_init();
        menu_bar_init();
        hmi_init();
        motion_control_init();
        while(globals->quit == false)
        {
            globals->quit = !Xrender_tick();
            motion_control_tick();
            ns_mgr_poll(&mgr, 1);
        }
        LOG_F(INFO, "Shutting down!");
        Xrender_close();
        ns_mgr_free(&mgr);
        delete globals;
    }
    return 0;
}