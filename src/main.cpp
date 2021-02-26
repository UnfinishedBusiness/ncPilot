#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "application.h"
#include "EasyRender/EasyRender.h"
#include "EasyRender/logging/loguru.h"
#include "EasyRender/gui/imgui.h"
#include "menu_bar/menu_bar.h"

global_variables_t *globals;

/*void view_matrix(PrimativeContainer *p)
{
    p->properties->scale = globals->zoom;
    p->properties->offset[0] = globals->pan.x;
    p->properties->offset[1] = globals->pan.y;
}*/
/*void zoom_handle(nlohmann::json e)
{
    LOG_F(INFO, "%s", e.dump().c_str());
    double_point_t matrix_mouse = globals->renderer->GetWindowMousePosition();
    matrix_mouse.x = (matrix_mouse.x - globals->pan.x) / globals->zoom;
    matrix_mouse.y = (matrix_mouse.y - globals->pan.y) / globals->zoom;
    if ((float)e["scroll"] > 0)
    {
        double old_zoom = globals->zoom;
        globals->zoom += globals->zoom * 0.125;
        if (globals->zoom > 1000000)
        {
            globals->zoom = 1000000;
        }
        double scalechange = old_zoom - globals->zoom;
        globals->pan.x += matrix_mouse.x * scalechange;
        globals->pan.y += matrix_mouse.y * scalechange; 
    }
    else
    {
        double old_zoom = globals->zoom;
        globals->zoom += globals->zoom * -0.125;
        if (globals->zoom < 0.00001)
        {
            globals->zoom = 0.00001;
        }
        double scalechange = old_zoom - globals->zoom;
        globals->pan.x += matrix_mouse.x * scalechange;
        globals->pan.y += matrix_mouse.y * scalechange; 
    }
}*/
void init_preferences()
{
    std::ifstream preferences_file(globals->renderer->GetConfigDirectory() + "preferences.json");
    if (preferences_file.is_open())
    {
        std::string preference_json_string((std::istreambuf_iterator<char>(preferences_file)), std::istreambuf_iterator<char>());
        nlohmann::json preferences = nlohmann::json::parse(preference_json_string.c_str());
        try
        {
            LOG_F(INFO, "Found %s!", std::string(globals->renderer->GetConfigDirectory() + "preferences.json").c_str());
            globals->preferences.background_color[0] = (double)preferences["background_color"]["r"];
            globals->preferences.background_color[1] = (double)preferences["background_color"]["g"];
            globals->preferences.background_color[2] = (double)preferences["background_color"]["b"];
            globals->preferences.machine_plane_color[0] = (double)preferences["machine_plane_color"]["r"];
            globals->preferences.machine_plane_color[1] = (double)preferences["machine_plane_color"]["g"];
            globals->preferences.machine_plane_color[2] = (double)preferences["machine_plane_color"]["b"];
            globals->preferences.cuttable_plane_color[0] = (double)preferences["cuttable_plane_color"]["r"];
            globals->preferences.cuttable_plane_color[1] = (double)preferences["cuttable_plane_color"]["g"];
            globals->preferences.cuttable_plane_color[2] = (double)preferences["cuttable_plane_color"]["b"];
            globals->preferences.window_size[0] = (double)preferences["window_width"];
            globals->preferences.window_size[1] = (double)preferences["window_height"];
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing preferences file!");
        }
    }
    else
    {
        LOG_F(WARNING, "Preferences file does not exist, creating it!");
        globals->preferences.background_color[0] = 8.0f;
        globals->preferences.background_color[1] = 14.0f;
        globals->preferences.background_color[2] = 84.0f;
        globals->preferences.machine_plane_color[0] = 100.0f;
        globals->preferences.machine_plane_color[1] = 100.0f;
        globals->preferences.machine_plane_color[2] = 100.0f;
        globals->preferences.cuttable_plane_color[0] = 151.0f;
        globals->preferences.cuttable_plane_color[1] = 5.0f;
        globals->preferences.cuttable_plane_color[2] = 5.0f;
        globals->preferences.window_size[0] = 500;
        globals->preferences.window_size[1] = 400;
    }

    std::ifstream json_file(globals->renderer->GetConfigDirectory() + "machine_parameters.json");
    if (json_file.is_open())
    {
        std::string json_string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
        nlohmann::json parameters = nlohmann::json::parse(json_string.c_str());
        try
        {
            LOG_F(INFO, "Found %s!", std::string(globals->renderer->GetConfigDirectory() + "machine_parameters.json").c_str());
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
            globals->machine_parameters.arc_stablization_time = (float)parameters["arc_stablization_time"];
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
        LOG_F(WARNING, "%s does not exist, using default parameters!", std::string(globals->renderer->GetConfigDirectory() + "machine_parameters.json").c_str());
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
        globals->machine_parameters.arc_stablization_time = 2000;
        globals->machine_parameters.floating_head_backlash = 0.200f;
        globals->machine_parameters.z_probe_feedrate = 40.0f;
        globals->machine_parameters.axis_invert[0] = true;
        globals->machine_parameters.axis_invert[1] = true;
        globals->machine_parameters.axis_invert[2] = true;
        globals->machine_parameters.axis_invert[3] = true;
    }
}
void log_uptime()
{
    unsigned long m = (EasyRender::Millis() - globals->start_timestamp);
    unsigned long seconds=(m/1000)%60;
    unsigned long minutes=(m/(1000*60))%60;
    unsigned long hours=(m/(1000*60*60))%24;
    LOG_F(INFO, "Shutting down, Adding Uptime: %luh %lum %lus to total", hours, minutes, seconds);
    std::ifstream uptime_file(globals->renderer->GetConfigDirectory() + "uptime.json");
    if (uptime_file.is_open())
    {
        std::string uptime_json_string((std::istreambuf_iterator<char>(uptime_file)), std::istreambuf_iterator<char>());
        nlohmann::json uptime_json = nlohmann::json::parse(uptime_json_string.c_str());
        try
        {
            hours += (unsigned long)uptime_json["hours"];
            minutes += (unsigned long)uptime_json["minutes"];
            seconds += (unsigned long)uptime_json["seconds"];
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing uptime file!");
        }
    }
    uptime_file.close();
    nlohmann::json uptime;
    uptime["hours"] = hours;
    uptime["minutes"] = minutes;
    uptime["seconds"] = seconds;
    std::ofstream out(globals->renderer->GetConfigDirectory() + "uptime.json");
    out << uptime.dump();
    out.close();
}
int main(int argc, char **argv)
{
    globals = new global_variables_t;
    globals->quit = false;
    globals->zoom = 1;
    globals->pan.x = 0;
    globals->pan.y = 0;
    globals->start_timestamp = EasyRender::Millis();
    globals->renderer = new EasyRender();
    globals->renderer->SetWindowTitle("ncPilot");
    globals->renderer->SetGuiIniFileName(globals->renderer->GetConfigDirectory() + "gui.ini");
    globals->renderer->SetGuiLogFileName(globals->renderer->GetConfigDirectory() + "gui.log");
    globals->renderer->SetMainLogFileName(globals->renderer->GetConfigDirectory() + "ncPilot.log");
    globals->renderer->SetShowFPS(true);
    init_preferences();
    globals->renderer->SetClearColor(globals->preferences.background_color[0], globals->preferences.background_color[1], globals->preferences.background_color[2]);
    globals->renderer->Init(argc, argv);


    //globals->renderer->PushEvent("up", "scroll", &zoom_handle);
    //globals->renderer->PushEvent("down", "scroll", &zoom_handle);

    menu_bar_init();
    while(globals->renderer->Poll(globals->quit))
    {
        //Do Stuff
    }
    log_uptime();
    globals->renderer->Close();
    delete globals->renderer;
    delete globals;
    return 0;
}