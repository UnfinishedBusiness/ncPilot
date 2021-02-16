#include <Xrender.h>
#include <dialogs/dialogs.h>
#include <application.h>
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include "json/json.h"
#include <string>
#include <fstream>
#include <streambuf>

Xrender_gui_t *preferences_window_handle;

void dialogs_file_open()
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
    if (ImGui::Button("OK"))
    {
        printf("Col[0] = %0.4f Col[1] = %.4f Col[2] = %.4f\n", globals->preferences.background_color[0] * 255, globals->preferences.background_color[1] * 255, globals->preferences.background_color[2] * 255);
        globals->Xcore->data["clear_color"]["r"] = globals->preferences.background_color[0] * 255;
        globals->Xcore->data["clear_color"]["g"] = globals->preferences.background_color[1] * 255;
        globals->Xcore->data["clear_color"]["b"] = globals->preferences.background_color[2] * 255;

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