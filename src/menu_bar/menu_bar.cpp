#include <Xrender.h>
#include <application.h>
#include "menu_bar/menu_bar.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "dialogs/dialogs.h"
#include <stdio.h>
#include <iostream>

Xrender_gui_t *menu_bar;

void menu_bar_render()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", ""))
            {
                std::string path = ".";
                std::ifstream f(Xrender_get_config_dir("ncPilot") + "last_gcode_open_path.conf");
                if (f.is_open())
                {
                    std::string p((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                    path = p;
                }
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".nc", path.c_str());
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Check for Updates", ""))
            {

            }
            if (ImGui::MenuItem("Flash Firmware", ""))
            {

            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close", ""))
            { 
                globals->quit = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Preferences", ""))
            {
                dialogs_show_preferences(true);
            }
            if (ImGui::MenuItem("Machine Parameters", ""))
            {
                //dialogs_show_machine_parameters(true); 
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
void menu_bar_init()
{
    menu_bar = Xrender_push_gui(true, menu_bar_render);
}