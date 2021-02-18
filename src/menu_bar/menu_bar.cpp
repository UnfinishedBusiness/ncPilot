#include <Xrender.h>
#include <application.h>
#include "menu_bar/menu_bar.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "dialogs/dialogs.h"
#include "logging/loguru.h"
#include "debug/debug.h"
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
                LOG_F(INFO, "File->Open");
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
            if (ImGui::MenuItem("Close", ""))
            {
                LOG_F(INFO, "Edit->Close");
                globals->quit = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Preferences", ""))
            {
                LOG_F(INFO, "Edit->Preferences");
                dialogs_show_preferences(true);
            }
            if (ImGui::MenuItem("Machine Parameters", ""))
            {
                LOG_F(INFO, "Edit->Machine Parameters");
                dialogs_show_machine_parameters(true);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools"))
        {
            if (ImGui::MenuItem("Check for Updates", ""))
            {
                LOG_F(INFO, "Tools->Checking for Updates");
            }
            if (ImGui::MenuItem("Flash Firmware", ""))
            {
                LOG_F(INFO, "Tools->Flash Firmware");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("CAD", ""))
            {
                LOG_F(INFO, "Tools->CAD");
            }
            if (ImGui::MenuItem("CAM", ""))
            {
                LOG_F(INFO, "Tools->CAM");
            }
            if (ImGui::MenuItem("NC", ""))
            {
                LOG_F(INFO, "Tools->NC");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Remote Support", ""))
            {
                LOG_F(INFO, "Help->Remote Support");
            }
            if (ImGui::MenuItem("Documentation", "")) 
            {
                LOG_F(INFO, "Help->Documentation");
            }
            if (ImGui::MenuItem("About", "")) 
            {
                LOG_F(INFO, "Help->About");
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