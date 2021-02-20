#include <Xrender.h>
#include <dialogs/dialogs.h>
#include <application.h>
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "logging/loguru.h"
#include "json/json.h"
#include "gcode/gcode.h"
#include "motion_control/motion_control.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
#include <iterator>

Xrender_gui_t *thc_window_handle;
Xrender_gui_t *preferences_window_handle;
Xrender_gui_t *machine_parameters_window_handle;
Xrender_gui_t *progress_window_handle;
float progress = 0.0f;
Xrender_gui_t *info_window_handle;
std::string info;

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
        motion_controller_save_machine_parameters();
        motion_controller_trigger_reset();
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

void dialogs_show_info_window(bool s)
{
    info_window_handle->visable = s;
}
void dialogs_set_info_value(std::string i)
{
    info = i;
    LOG_F(WARNING, "Info Window => %s", info.c_str());
    dialogs_show_info_window(true);
}
void dialogs_info_window()
{
    ImGui::Begin("Info", &info_window_handle->visable, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("%s", info.c_str());
    if (ImGui::Button("Close"))
    {
        dialogs_show_info_window(false);
    }
    ImGui::End();
}

void dialogs_show_thc_window(bool s)
{
    thc_window_handle->visable = s;
}
void dialogs_thc_window()
{
    ImGui::Begin("Torch Height Control", &thc_window_handle->visable, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Smart THC mode automatically sets the \"set voltage\" that is measured shortly\nafter the torch pierces and moves negative to cut height.\n By doing this, the THC should maintain approximately\nthe same cut height as is set in the cutting parameters.");
    ImGui::BulletText("Smart THC Should not be used on thin materials that warp when the torch touches off!");
    ImGui::Separator();
    ImGui::Checkbox("Turn on Auto THC Setting Mode", &globals->machine_parameters.smart_thc_on);
    ImGui::Separator();
    ImGui::Text("When Smart THC is off (Not Checked) the THC set voltage set below will be used");
    ImGui::Text("0 = THC OFF, Max value is 1024. Press Tab to manually enter a value");
    ImGui::Separator();
    ImGui::SliderInt("Set Voltage", &globals->machine_parameters.thc_set_value, 0, 1024);
    if (ImGui::Button("Close"))
    {
        dialogs_show_thc_window(false);
    }
    ImGui::End();
}

void dialogs_init()
{
    Xrender_push_gui(true, dialogs_file_open);
    preferences_window_handle = Xrender_push_gui(false, dialogs_preferences);
    machine_parameters_window_handle = Xrender_push_gui(false, dialogs_machine_parameters);
    progress_window_handle = Xrender_push_gui(false, dialogs_progress_window);
    info_window_handle = Xrender_push_gui(false, dialogs_info_window);
    thc_window_handle = Xrender_push_gui(false, dialogs_thc_window);
}