#include "Xrender.h"
#include "application.h"
#include "json/json.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "debug/debug.h"
#include "event_handling/event_handling.h"
#include "menu_bar/menu_bar.h"
#include "dialogs/dialogs.h"
#include "utility/utility.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

global_variables_t *globals;

nlohmann::json view_matrix(nlohmann::json data)
{
    nlohmann::json new_data = data;
    if (data["type"] == "line")
    {
        new_data["start"]["x"] = ((double)data["start"]["x"] * globals->zoom) + globals->pan.x;
        new_data["start"]["y"] = ((double)data["start"]["y"] * globals->zoom) + globals->pan.y;
        new_data["end"]["x"] = ((double)data["end"]["x"] * globals->zoom) + globals->pan.x;
        new_data["end"]["y"] = ((double)data["end"]["y"] * globals->zoom) + globals->pan.y;
    }
    if (data["type"] == "arc" || data["type"] == "circle")
    {
        new_data["center"]["x"] = ((double)data["center"]["x"] * globals->zoom) + globals->pan.x;
        new_data["center"]["y"] = ((double)data["center"]["y"]* globals->zoom) + globals->pan.y;
        new_data["radius"] = ((double)data["radius"] * globals->zoom);
    }
    if (data["type"] == "box")
    {
        new_data["tl"]["x"] = ((double)data["tl"]["x"] * globals->zoom) + globals->pan.x;
        new_data["tl"]["y"] = ((double)data["tl"]["y"] * globals->zoom) + globals->pan.y;
        new_data["br"]["x"] = ((double)data["br"]["x"] * globals->zoom) + globals->pan.x;
        new_data["br"]["y"] = ((double)data["br"]["y"] * globals->zoom) + globals->pan.y;
    }
    return new_data;
}
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
            printf("Error parsing preferences file!\n");
        }
    }
    else
    {
        //printf("Preferences file does not exist!\n");
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
}
int main()
{
    printf("Config directory: %s\n", Xrender_get_config_dir("ncPilot").c_str());
    if (!utility_dir_exists(Xrender_get_config_dir("ncPilot").c_str()))
    {
        #if defined(_WIN32)
            _mkdir(Xrender_get_config_dir("ncPilot").c_str()); // can be used on Windows
        #else 
            mkdir(Xrender_get_config_dir("ncPilot").c_str(), 0700);
        #endif
    }
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

        globals->machine_plane = Xrender_push_box({
            {"tl", {
                {"x", 0},
                {"y", 45}
            }},
            {"br", {
                {"x", 45},
                {"y", 0}
            }},
            {"radius", 0},
            {"zindex", -20},
            {"color", {
                {"r", globals->preferences.machine_plane_color[0] * 255},
                {"g", globals->preferences.machine_plane_color[1] * 255},
                {"b", globals->preferences.machine_plane_color[2] * 255},
                {"a", 255}
            }},
        });
        globals->machine_plane->matrix_data = view_matrix;

        globals->cuttable_plane = Xrender_push_box({
            {"tl", {
                {"x", 1},
                {"y", 41}
            }},
            {"br", {
                {"x", 41},
                {"y", 1}
            }},
            {"radius", 0},
            {"zindex", -10},
            {"color", {
                {"r", globals->preferences.cuttable_plane_color[0] * 255},
                {"g", globals->preferences.cuttable_plane_color[1] * 255},
                {"b", globals->preferences.cuttable_plane_color[2] * 255},
                {"a", 255}
            }},
        });
        globals->cuttable_plane->matrix_data = view_matrix;
        

        while(Xrender_tick() && globals->quit == false)
        {
            //Running!
        }
        Xrender_close();
        delete globals;
    }
    return 0;
}