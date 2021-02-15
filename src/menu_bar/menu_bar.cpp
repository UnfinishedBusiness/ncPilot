#include <Xrender.h>
#include <application.h>
#include "menu_bar/menu_bar.h"
#include "gui/imgui.h"
#include "gui/ImGuiFileDialog.h"
#include <stdio.h>
#include <iostream>

Xrender_gui_t *menu_bar;

void menu_bar_render()
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
            if (ImGui::MenuItem("Close", "")) { globals->quit = true; }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            //if (ImGui::MenuItem("Preferences", "")) { preferences_window->visable = true; }
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
void menu_bar_init()
{
    menu_bar = Xrender_push_gui(true, menu_bar_render);
}