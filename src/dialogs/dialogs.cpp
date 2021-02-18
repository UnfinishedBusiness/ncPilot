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
Xrender_gui_t *machine_parameters_window_handle;
Xrender_gui_t *progress_window_handle;
float progress = 0.0f;

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
    ImGui::Begin("Preferences", &preferences_window_handle->visable, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::ColorEdit3("Background Color", globals->preferences.background_color);
    ImGui::ColorEdit3("Machine Plane Color", globals->preferences.machine_plane_color);
    ImGui::ColorEdit3("Cuttable Plane Color", globals->preferences.cuttable_plane_color);
    ImGui::Spacing();
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
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        dialogs_show_preferences(false);
    }
    ImGui::End();
}

void dialogs_show_machine_parameters(bool s)
{
    machine_parameters_window_handle->visable = s;
}
void dialogs_machine_parameters()
{
    ImGui::Begin("Machine Parameters", &machine_parameters_window_handle->visable, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::InputFloat3("Machine Extents (X, Y, Z)", globals->machine_parameters.machine_extents);
    ImGui::InputFloat4("Cutting Extents (X1, Y1, X2, Y2)", globals->machine_parameters.cutting_extents);
    ImGui::Text("Scale is in steps per your desired units. E.G. To use machine in\nInches, set scales to steps per inch");
    ImGui::InputFloat3("Axis Scale (X, Y, Z)", globals->machine_parameters.axis_scale);
    ImGui::Checkbox("Invert X", &globals->machine_parameters.axis_invert[0]);
    ImGui::SameLine();
    ImGui::Checkbox("Invert Y1", &globals->machine_parameters.axis_invert[1]);
    ImGui::SameLine();
    ImGui::Checkbox("Invert Y2", &globals->machine_parameters.axis_invert[2]);
    ImGui::SameLine();
    ImGui::Checkbox("Invert Z", &globals->machine_parameters.axis_invert[3]);
    ImGui::InputFloat3("Max Velocity (X, Y, Z)", globals->machine_parameters.max_vel);
    ImGui::InputFloat3("Max Acceleration (X, Y, Z)", globals->machine_parameters.max_accel);
    ImGui::InputFloat("Floating Head Takup", &globals->machine_parameters.floating_head_backlash);
    ImGui::InputFloat("Z Probe Feed", &globals->machine_parameters.z_probe_feedrate);
    ImGui::Spacing();
    if (ImGui::Button("OK"))
    {
        //Write preferences to file
        nlohmann::json preferences;
        preferences["machine_extents"]["x"] = globals->machine_parameters.machine_extents[0];
        preferences["machine_extents"]["y"] = globals->machine_parameters.machine_extents[1];
        preferences["machine_extents"]["z"] = globals->machine_parameters.machine_extents[2];
        preferences["cutting_extents"]["x1"] = globals->machine_parameters.cutting_extents[0];
        preferences["cutting_extents"]["y1"] = globals->machine_parameters.cutting_extents[1];
        preferences["cutting_extents"]["x2"] = globals->machine_parameters.cutting_extents[2];
        preferences["cutting_extents"]["y2"] = globals->machine_parameters.cutting_extents[3];
        preferences["axis_scale"]["x"] = globals->machine_parameters.axis_scale[0];
        preferences["axis_scale"]["y"] = globals->machine_parameters.axis_scale[1];
        preferences["axis_scale"]["z"] = globals->machine_parameters.axis_scale[2];
        preferences["max_vel"]["x"] = globals->machine_parameters.max_vel[0];
        preferences["max_vel"]["y"] = globals->machine_parameters.max_vel[1];
        preferences["max_vel"]["z"] = globals->machine_parameters.max_vel[2];
        preferences["max_accel"]["x"] = globals->machine_parameters.max_accel[0];
        preferences["max_accel"]["y"] = globals->machine_parameters.max_accel[1];
        preferences["max_accel"]["z"] = globals->machine_parameters.max_accel[2];
        preferences["junction_deviation"] = globals->machine_parameters.junction_deviation;
        preferences["floating_head_backlash"] = globals->machine_parameters.floating_head_backlash;
        preferences["z_probe_feedrate"] = globals->machine_parameters.z_probe_feedrate;
        preferences["axis_invert"]["x"] = globals->machine_parameters.axis_invert[0];
        preferences["axis_invert"]["y1"] = globals->machine_parameters.axis_invert[1];
        preferences["axis_invert"]["y2"] = globals->machine_parameters.axis_invert[2];
        preferences["axis_invert"]["z"] = globals->machine_parameters.axis_invert[3];
        globals->machine_plane->data["tl"] = {{"x", 0},{"y", globals->machine_parameters.machine_extents[1]}};
        globals->machine_plane->data["br"] = {{"x", globals->machine_parameters.machine_extents[0]},{"y", 0}};
        globals->cuttable_plane->data["tl"] = {{"x", globals->machine_parameters.cutting_extents[0]},{"y", globals->machine_parameters.machine_extents[1]+globals->machine_parameters.cutting_extents[3]}};
        globals->cuttable_plane->data["br"] = {{"x", globals->machine_parameters.machine_extents[0]+globals->machine_parameters.cutting_extents[2]},{"y", globals->machine_parameters.cutting_extents[1]}};
        std::ofstream out(Xrender_get_config_dir("ncPilot") + "machine_parameters.json");
        out << preferences.dump();
        out.close();
        dialogs_show_machine_parameters(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        dialogs_show_machine_parameters(false);
    }
    ImGui::End();
}

void dialogs_show_progress_window(bool s)
{
    progress_window_handle->visable = s;
}
void dialogs_set_progress_value(float p)
{
    progress = p;
}
void dialogs_progress_window()
{
    ImGui::Begin("Progress", &progress_window_handle->visable, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::ProgressBar(progress, ImVec2(0.0f, 0.0f));
    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::Text("Progress");
    ImGui::End();
}

void dialogs_init()
{
    Xrender_push_gui(true, dialogs_file_open);
    preferences_window_handle = Xrender_push_gui(false, dialogs_preferences);
    machine_parameters_window_handle = Xrender_push_gui(false, dialogs_machine_parameters);
    progress_window_handle = Xrender_push_gui(false, dialogs_progress_window);
}