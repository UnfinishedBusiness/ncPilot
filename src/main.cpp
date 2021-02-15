#include "Xrender.h"
#include "application.h"
#include "json/json.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "debug/debug.h"
#include "event_handling/event_handling.h"
#include "menu_bar/menu_bar.h"

global_variables_t *globals;

Xrender_core_t *Xcore;


float col1[3] = { 1.0f, 0.0f, 0.2f };
Xrender_gui_t *preferences_window;

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
    return new_data;
}

void _preference_window()
{
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Preferences", &preferences_window->visable, 0);
    ImGui::ColorEdit3("Background Color", col1);
    if (ImGui::Button("OK"))
    {
        printf("Col[0] = %0.4f Col[1] = %.4f Col[2] = %.4f\n", col1[0] * 255, col1[1] * 255, col1[2] * 255);
        Xcore->data["clear_color"]["r"] = col1[0] * 255;
        Xcore->data["clear_color"]["g"] = col1[1] * 255;
        Xcore->data["clear_color"]["b"] = col1[2] * 255;
        preferences_window->visable = false;
    }
    if (ImGui::Button("Cancel"))
    {
        preferences_window->visable = false;
    }
    ImGui::End();
}
int main()
{
    //printf("App Config Dir = %s\n", Xrender_get_config_dir("test1").c_str());
    if (Xrender_init({{"window_title", "ncPilot"}, {"maximize", false}, {"window_width", 1200}, {"window_height", 700}, {"clear_color", { {"r", 8}, {"g", 14}, {"b", 84}, {"a", 255}}}}))
    {
        /*
            Initialize global variables
        */
        globals = new global_variables_t;
        globals->zoom = 5;
        globals->pan.x = 0;
        globals->pan.y = 0;
        globals->mouse_pos_in_screen_coordinates = {0, 0};
        globals->mouse_pos_in_matrix_coordinates = {0, 0};



        Xcore = Xrender_get_core_variables();
        debug_init(Xcore);
        event_handling_init();
        menu_bar_init();
        

        preferences_window = Xrender_push_gui(false, _preference_window);
        
                
        while(Xrender_tick() && globals->quit == false)
        {
            //Running!
        }
        Xrender_close();
        delete globals;
    }
    return 0;
}