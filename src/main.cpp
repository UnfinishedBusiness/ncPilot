#include "Xrender.h"
#include "json/json.h"
#include "serial/serial.h"
#include "geometry/geometry.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "gui/TextEditor.h"
#include "stk500/stk500.h"

serial::Serial serial_port;
bool quite = false;
double zoom = 1;
Xrender_core_t *Xcore;
Xrender_gui_t *menu_bar;
double_point_t pan = {0, 0};
double_point_t mouse_pos_in_screen_coordinates = {0, 0};
double_point_t mouse_pos_in_matrix_coordinates = {0, 0};

float col1[3] = { 1.0f, 0.0f, 0.2f };
Xrender_gui_t *preferences_window;

void mouse_motion(nlohmann::json e)
{
    //printf("%s\n", e.dump().c_str());
    mouse_pos_in_screen_coordinates = {(double)e["pos"]["x"], (double)e["pos"]["y"]};
    mouse_pos_in_matrix_coordinates = {
        (mouse_pos_in_screen_coordinates.x - pan.x) / zoom,
        (mouse_pos_in_screen_coordinates.y - pan.y) / zoom
    };
    //printf("Mouse pos: (%.4f, %.4f)\n", mouse_pos_in_matrix_coordinates.x, mouse_pos_in_matrix_coordinates.y);
}
nlohmann::json view_matrix(nlohmann::json data)
{
    nlohmann::json new_data = data;
    if (data["type"] == "line")
    {
        new_data["start"]["x"] = ((double)data["start"]["x"] * zoom) + pan.x;
        new_data["start"]["y"] = ((double)data["start"]["y"] * zoom) + pan.y;
        new_data["end"]["x"] = ((double)data["end"]["x"] * zoom) + pan.x;
        new_data["end"]["y"] = ((double)data["end"]["y"] * zoom) + pan.y;
    }
    if (data["type"] == "arc" || data["type"] == "circle")
    {
        new_data["center"]["x"] = ((double)data["center"]["x"] * zoom) + pan.x;
        new_data["center"]["y"] = ((double)data["center"]["y"]* zoom) + pan.y;
        new_data["radius"] = ((double)data["radius"] * zoom);
    }
    return new_data;
}
void zoom_in(nlohmann::json e)
{
    double old_zoom = zoom;
    zoom += zoom * 0.125;
    if (zoom > 1000000)
    {
        zoom = 1000000;
    }
    double scalechange = old_zoom - zoom;
    pan.x += mouse_pos_in_matrix_coordinates.x * scalechange;
    pan.y += mouse_pos_in_matrix_coordinates.y * scalechange; 
}
void zoom_out(nlohmann::json e)
{
    double old_zoom = zoom;
    zoom += zoom * -0.125;
    if (zoom < 0.00001)
    {
        zoom = 0.00001;
    }
    double scalechange = old_zoom - zoom;
    pan.x += mouse_pos_in_matrix_coordinates.x * scalechange;
    pan.y += mouse_pos_in_matrix_coordinates.y * scalechange; 
}
void _menu_bar()
{
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(600, 500))) 
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            printf("File Path: %s\n", filePathName.c_str());
            //Xrender_parse_dxf_file(filePathName, handle_dxf);
        }
                    // close
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                //ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".dxf", ".");
            }
            if (ImGui::MenuItem("Close", "")) { quite = true; }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Preferences", "")) { preferences_window->visable = true; }
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
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
    if (Xrender_init({{"window_title", "ncPilot"}, {"maximize", false}, {"clear_color", { {"r", 8}, {"g", 14}, {"b", 84}, {"a", 255}}}}))
    {
        Xrender_push_key_event({"up", "scroll", zoom_in});
        Xrender_push_key_event({"down", "scroll", zoom_out});
        Xrender_push_key_event({"none", "mouse_move", mouse_motion});
        menu_bar = Xrender_push_gui(true, _menu_bar);
        preferences_window = Xrender_push_gui(false, _preference_window);
        Xcore = Xrender_get_core_variables();

        Xrender_object_t *line = Xrender_push_line({
            {"start", {
                {"x", 5},
                {"y", 5}
            }},
            {"end", {
                {"x", 10},
                {"y", 10}
            }},
            {"color", {
                {"r", 255},
                {"g", 255},
                {"b", 255},
                {"a", 255},
            }},
        });
        line->matrix_data = view_matrix;
        line->mouse_callback = NULL;
        
        while(Xrender_tick() && quite == false)
        {
            //Running!
        }
        Xrender_close();
    }
    return 0;
}