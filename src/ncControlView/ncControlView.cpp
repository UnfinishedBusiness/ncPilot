#include "ncControlView.h"
#include <EasyRender/EasyRender.h>
#include <EasyRender/logging/loguru.h>
#include <EasyRender/gui/imgui.h>
#include "menu_bar/menu_bar.h"
#include "dialogs/dialogs.h"
#include "hmi/hmi.h"
#include "motion_control/motion_control.h"

void zoom_event_handle(nlohmann::json e)
{
    //LOG_F(INFO, "%s", e.dump().c_str());
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
}

void ncControlView::PreInit()
{
    globals->renderer->SetCurrentView("ncControlView");
    std::ifstream preferences_file(globals->renderer->GetConfigDirectory() + "preferences.json");
    if (preferences_file.is_open())
    {
        std::string preference_json_string((std::istreambuf_iterator<char>(preferences_file)), std::istreambuf_iterator<char>());
        nlohmann::json preferences = nlohmann::json::parse(preference_json_string.c_str());
        try
        {
            LOG_F(INFO, "Found %s!", std::string(globals->renderer->GetConfigDirectory() + "preferences.json").c_str());
            this->preferences.background_color[0] = (double)preferences["background_color"]["r"];
            this->preferences.background_color[1] = (double)preferences["background_color"]["g"];
            this->preferences.background_color[2] = (double)preferences["background_color"]["b"];
            this->preferences.machine_plane_color[0] = (double)preferences["machine_plane_color"]["r"];
            this->preferences.machine_plane_color[1] = (double)preferences["machine_plane_color"]["g"];
            this->preferences.machine_plane_color[2] = (double)preferences["machine_plane_color"]["b"];
            this->preferences.cuttable_plane_color[0] = (double)preferences["cuttable_plane_color"]["r"];
            this->preferences.cuttable_plane_color[1] = (double)preferences["cuttable_plane_color"]["g"];
            this->preferences.cuttable_plane_color[2] = (double)preferences["cuttable_plane_color"]["b"];
            this->preferences.window_size[0] = (double)preferences["window_width"];
            this->preferences.window_size[1] = (double)preferences["window_height"];
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing preferences file!");
        }
    }
    else
    {
        LOG_F(WARNING, "Preferences file does not exist, creating it!");
        this->preferences.background_color[0] = 8.0f / 255.0f;
        this->preferences.background_color[1] = 14.0f / 255.0f;
        this->preferences.background_color[2] = 84.0f / 255.0f;
        this->preferences.machine_plane_color[0] = 100.0f / 255.0f;
        this->preferences.machine_plane_color[1] = 100.0f / 255.0f;
        this->preferences.machine_plane_color[2] = 100.0f / 255.0f;
        this->preferences.cuttable_plane_color[0] = 151.0f / 255.0f;
        this->preferences.cuttable_plane_color[1] = 5.0f / 255.0f;
        this->preferences.cuttable_plane_color[2] = 5.0f / 255.0f;
        this->preferences.window_size[0] = 800;
        this->preferences.window_size[1] = 600;
    }

    std::ifstream json_file(globals->renderer->GetConfigDirectory() + "machine_parameters.json");
    if (json_file.is_open())
    {
        std::string json_string((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
        nlohmann::json parameters = nlohmann::json::parse(json_string.c_str());
        try
        {
            LOG_F(INFO, "Found %s!", std::string(globals->renderer->GetConfigDirectory() + "machine_parameters.json").c_str());
            this->machine_parameters.work_offset[0] = (float)parameters["work_offset"]["x"];
            this->machine_parameters.work_offset[1] = (float)parameters["work_offset"]["y"];
            this->machine_parameters.work_offset[2] = (float)parameters["work_offset"]["z"];
            this->machine_parameters.machine_extents[0] = (float)parameters["machine_extents"]["x"];
            this->machine_parameters.machine_extents[1] = (float)parameters["machine_extents"]["y"];
            this->machine_parameters.machine_extents[2] = (float)parameters["machine_extents"]["z"];
            this->machine_parameters.cutting_extents[0] = (float)parameters["cutting_extents"]["x1"];
            this->machine_parameters.cutting_extents[1] = (float)parameters["cutting_extents"]["y1"];
            this->machine_parameters.cutting_extents[2] = (float)parameters["cutting_extents"]["x2"];
            this->machine_parameters.cutting_extents[3] = (float)parameters["cutting_extents"]["y2"];
            this->machine_parameters.axis_scale[0] = (float)parameters["axis_scale"]["x"];
            this->machine_parameters.axis_scale[1] = (float)parameters["axis_scale"]["y"];
            this->machine_parameters.axis_scale[2] = (float)parameters["axis_scale"]["z"];
            this->machine_parameters.max_vel[0] = (float)parameters["max_vel"]["x"];
            this->machine_parameters.max_vel[1] = (float)parameters["max_vel"]["y"];
            this->machine_parameters.max_vel[2] = (float)parameters["max_vel"]["z"];
            this->machine_parameters.max_accel[0] = (float)parameters["max_accel"]["x"];
            this->machine_parameters.max_accel[1] = (float)parameters["max_accel"]["y"];
            this->machine_parameters.max_accel[2] = (float)parameters["max_accel"]["z"];
            this->machine_parameters.junction_deviation = (float)parameters["junction_deviation"];
            this->machine_parameters.arc_stablization_time = (float)parameters["arc_stablization_time"];
            this->machine_parameters.floating_head_backlash = (float)parameters["floating_head_backlash"];
            this->machine_parameters.z_probe_feedrate = (float)parameters["z_probe_feedrate"];
            this->machine_parameters.axis_invert[0] = (bool)parameters["axis_invert"]["x"];
            this->machine_parameters.axis_invert[1] = (bool)parameters["axis_invert"]["y1"];
            this->machine_parameters.axis_invert[2] = (bool)parameters["axis_invert"]["y2"];
            this->machine_parameters.axis_invert[3] = (bool)parameters["axis_invert"]["z"];
        }
        catch(...)
        {
            LOG_F(WARNING, "Error parsing Machine Parameters file!");
        }
    }
    else
    {
        LOG_F(WARNING, "%s does not exist, using default parameters!", std::string(globals->renderer->GetConfigDirectory() + "machine_parameters.json").c_str());
        this->machine_parameters.work_offset[0] = 0.0f;
        this->machine_parameters.work_offset[1] = 0.0f;
        this->machine_parameters.work_offset[2] = 0.0f;
        this->machine_parameters.machine_extents[0] = 48.0f;
        this->machine_parameters.machine_extents[1] = 96.0f;
        this->machine_parameters.machine_extents[2] = -6.0f;
        this->machine_parameters.cutting_extents[0] = 1.0f;
        this->machine_parameters.cutting_extents[1] = 1.0f;
        this->machine_parameters.cutting_extents[2] = -1.0f;
        this->machine_parameters.cutting_extents[3] = -1.0f;
        this->machine_parameters.axis_scale[0] = 1016.002f;
        this->machine_parameters.axis_scale[1] = 1016.002f;
        this->machine_parameters.axis_scale[2] = 1016.002f;
        this->machine_parameters.max_vel[0] = 230.0f;
        this->machine_parameters.max_vel[1] = 400.0f;
        this->machine_parameters.max_vel[2] = 200.0f;
        this->machine_parameters.max_accel[0] = 8.0f;
        this->machine_parameters.max_accel[1] = 8.0f;
        this->machine_parameters.max_accel[2] = 20.0f;
        this->machine_parameters.junction_deviation = 0.0005f;
        this->machine_parameters.arc_stablization_time = 2000;
        this->machine_parameters.floating_head_backlash = 0.200f;
        this->machine_parameters.z_probe_feedrate = 40.0f;
        this->machine_parameters.axis_invert[0] = true;
        this->machine_parameters.axis_invert[1] = true;
        this->machine_parameters.axis_invert[2] = true;
        this->machine_parameters.axis_invert[3] = true;
    }
    this->view_matrix = &hmi_view_matrix;
}
void ncControlView::Init()
{
    globals->renderer->PushEvent("up", "scroll", &zoom_event_handle);
    globals->renderer->PushEvent("down", "scroll", &zoom_event_handle);
    menu_bar_init();
    dialogs_init();
    motion_control_init();
    hmi_init();
}
void ncControlView::Tick()
{
    motion_control_tick();
}
void ncControlView::MakeActive()
{
    globals->renderer->SetCurrentView("ncControlView");
    globals->renderer->SetClearColor(globals->nc_control_view->preferences.background_color[0] * 255.0f, globals->nc_control_view->preferences.background_color[1] * 255.0f, globals->nc_control_view->preferences.background_color[2] * 255.0f);
}
void ncControlView::Close()
{
    
}