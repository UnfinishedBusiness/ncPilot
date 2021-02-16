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
            if (ImGui::MenuItem("Open", "CTRL+O"))
            {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".dxf", ".");
            }
            if (ImGui::MenuItem("Close", "")) { globals->quit = true; }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Preferences", "")) { dialogs_show_preferences(true); }
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