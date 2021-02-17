#include <Xrender.h>
#include <dialogs/dialogs.h>
#include <application.h>
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "logging/loguru.h"
#include "json/json.h"
#include "gcode/gcode.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

Xrender_gui_t *preferences_window_handle;

void dialogs_file_open()
{
    if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, ImVec2(600, 500))) 
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            LOG_F(INFO, "File Path: %s, File Path Name: %s", filePath.c_str(), filePathName.c_str());
            std::ofstream out(Xrender_get_config_dir("ncPilot") + "last_gcode_open_path.conf");
            out << filePath;
            out << "/";
            out.close();
            //Xrender_parse_dxf_file(filePathName, handle_dxf);
            if (gcode_open_file(filePathName))
            {
                Xrender_push_timer(0, &gcode_parse_timer);
            }
        }
        ImGuiFileDialog::Instance()->Close();
    }
}
void dialogs_show_preferences(bool s) 
{
    preferences_window_handle->visable = s;
}
void dialogs_preferences()
{
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Preferences", &preferences_window_handle->visable, 0);
    ImGui::ColorEdit3("Background Color", globals->preferences.background_color);
    ImGui::ColorEdit3("Machine Plane Color", globals->preferences.machine_plane_color);
    ImGui::ColorEdit3("Cuttable Plane Color", globals->preferences.cuttable_plane_color);
    if (ImGui::Button("OK"))
    {
        globals->Xcore->data["clear_color"]["r"] = globals->preferences.background_color[0] * 255;
        globals->Xcore->data["clear_color"]["g"] = globals->preferences.background_color[1] * 255;
        globals->Xcore->data["clear_color"]["b"] = globals->preferences.background_color[2] * 255;

        globals->machine_plane->data["color"]["r"] = globals->preferences.machine_plane_color[0] * 255;
        globals->machine_plane->data["color"]["g"] = globals->preferences.machine_plane_color[1] * 255;
        globals->machine_plane->data["color"]["b"] = globals->preferences.machine_plane_color[2] * 255;

        globals->cuttable_plane->data["color"]["r"] = globals->preferences.cuttable_plane_color[0] * 255;
        globals->cuttable_plane->data["color"]["g"] = globals->preferences.cuttable_plane_color[1] * 255;
        globals->cuttable_plane->data["color"]["b"] = globals->preferences.cuttable_plane_color[2] * 255;

        //Write preferences to file
        nlohmann::json preferences;
        preferences["background_color"] = globals->Xcore->data["clear_color"];
        preferences["machine_plane_color"] = globals->machine_plane->data["color"];
        preferences["cuttable_plane_color"] = globals->cuttable_plane->data["color"];
        std::ofstream out(Xrender_get_config_dir("ncPilot") + "preferences.json");
        out << preferences.dump();
        out.close();
        dialogs_show_preferences(false);
    }
    if (ImGui::Button("Cancel"))
    {
        dialogs_show_preferences(false);
    }
    ImGui::End();
}
void dialogs_init()
{
    Xrender_push_gui(true, dialogs_file_open);
    preferences_window_handle = Xrender_push_gui(false, dialogs_preferences);
}