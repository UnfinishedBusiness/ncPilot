#include "menu_bar.h"
#include <dialogs/dialogs.h>
#include <motion_control/motion_control.h>

EasyRender::EasyRenderGui *menu_bar;

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
                std::ifstream f(globals->renderer->GetConfigDirectory() + "last_gcode_open_path.conf");
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
            ImGui::Separator();
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
            ImGui::Separator();
            if (ImGui::MenuItem("Update Firmware", ""))
            {
                LOG_F(INFO, "Tools->Update Firmware");
                dialogs_set_info_value("Updating firmware on motion controller.\nThe screen will seem frozen during this process, please wait until it's complete!");
                globals->renderer->PushTimer(1000, &motion_control_update_firmware);
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Workbench"))
        {
            if (ImGui::MenuItem("Machine", ""))
            {
                LOG_F(INFO, "Workbench->Machine");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Design", ""))
            {
                LOG_F(INFO, "Workbench->Design");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Remote Support", ""))
            {
                LOG_F(INFO, "Help->Remote Support");
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Documentation", "")) 
            {
                LOG_F(INFO, "Help->Documentation");
            }
            ImGui::Separator();
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
    menu_bar = globals->renderer->PushGui(true, &menu_bar_render);
}