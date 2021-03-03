#include "jetCamView.h"
#include <EasyRender/EasyRender.h>
#include <EasyRender/logging/loguru.h>
#include <EasyRender/gui/imgui.h>
#include <EasyRender/gui/ImGuiFileDialog.h>

void jetCamView::ZoomEventCallback(nlohmann::json e)
{
    double_point_t matrix_mouse = globals->renderer->GetWindowMousePosition();
    matrix_mouse.x = (matrix_mouse.x - globals->pan.x) / globals->zoom;
    matrix_mouse.y = (matrix_mouse.y - globals->pan.y) / globals->zoom;
    if ((float)e["scroll"] > 0)
    {
        double old_zoom = globals->zoom;
        globals->zoom += globals->zoom * 0.125;
        if (globals->zoom > 1000000)
        {
            globals->zoom = 1000000;
        }
        double scalechange = old_zoom - globals->zoom;
        globals->pan.x += matrix_mouse.x * scalechange;
        globals->pan.y += matrix_mouse.y * scalechange;
    }
    else
    {
        double old_zoom = globals->zoom;
        globals->zoom += globals->zoom * -0.125;
        if (globals->zoom < 0.00001)
        {
            globals->zoom = 0.00001;
        }
        double scalechange = old_zoom - globals->zoom;
        globals->pan.x += matrix_mouse.x * scalechange;
        globals->pan.y += matrix_mouse.y * scalechange; 
    }
}
void jetCamView::ViewMatrixCallback(PrimativeContainer *p)
{
    if (p->type == "line")
    {
        p->properties->scale = globals->zoom;
        p->properties->offset[0] = globals->pan.x + (globals->nc_control_view->machine_parameters.work_offset[0] * globals->zoom);
        p->properties->offset[1] = globals->pan.y + (globals->nc_control_view->machine_parameters.work_offset[1] * globals->zoom);
    }
    if (p->type == "arc" || p->type == "circle")
    {
        if (p->properties->id == "torch_pointer")
        {
            p->properties->offset[0] = globals->pan.x;
            p->properties->offset[1] = globals->pan.y;
            p->properties->scale = globals->zoom;
            p->circle->radius = 5.0f / globals->zoom;
        }
        else
        {
            p->properties->offset[0] = globals->pan.x;
            p->properties->offset[1] = globals->pan.y;
            p->properties->scale = globals->zoom;
        }
    }
    if (p->type == "box")
    {
        p->properties->scale = globals->zoom;
        p->properties->offset[0] = globals->pan.x;
        p->properties->offset[1] = globals->pan.y;
    }
    if (p->type == "path")
    {
        if (p->properties->id == "gcode" || p->properties->id == "gcode_highlights")
        {
            p->properties->scale = globals->zoom;
            p->properties->offset[0] = globals->pan.x + (globals->nc_control_view->machine_parameters.work_offset[0] * globals->zoom);
            p->properties->offset[1] = globals->pan.y + (globals->nc_control_view->machine_parameters.work_offset[1] * globals->zoom);
        }
        if (p->properties->id == "gcode_arrows")
        {
            p->properties->scale = globals->zoom;
            p->properties->offset[0] = globals->pan.x + (globals->nc_control_view->machine_parameters.work_offset[0] * globals->zoom);
            p->properties->offset[1] = globals->pan.y + (globals->nc_control_view->machine_parameters.work_offset[1] * globals->zoom);
        }
    }
}
void jetCamView::MouseEventCallback(PrimativeContainer* c,nlohmann::json e)
{
    LOG_F(INFO, "%s", e.dump().c_str());
}
int last_hovered_items[2] = {-1, -1};
void jetCamView::RenderUI()
{
    if (ImGuiFileDialog::Instance()->Display("ImportPartDialog", ImGuiWindowFlags_NoCollapse, ImVec2(600, 500)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
            LOG_F(INFO, "File Path: %s, File Path Name: %s", filePath.c_str(), filePathName.c_str());
            std::ofstream out(globals->renderer->GetConfigDirectory() + "last_dxf_open_path.conf");
            out << filePath;
            out << "/";
            out.close();
            /*if (gcode_open_file(filePathName))
            {
                globals->renderer->PushTimer(0, &gcode_parse_timer);
            }*/
        }
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Import Part", ""))
            {
                LOG_F(INFO, "File->Open");
                std::string path = ".";
                std::ifstream f(globals->renderer->GetConfigDirectory() + "last_dxf_open_path.conf");
                if (f.is_open())
                {
                    std::string p((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                    path = p;
                }
                ImGuiFileDialog::Instance()->OpenDialog("ImportPartDialog", "Choose File", ".dxf", path.c_str());
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
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Workbench"))
        {
            if (ImGui::MenuItem("Machine Control", ""))
            {
                LOG_F(INFO, "Workbench->Machine Control");
                globals->nc_control_view->MakeActive();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("CAM Toolpaths", ""))
            {
                LOG_F(INFO, "Workbench->CAM Toolpaths");
                globals->jet_cam_view->MakeActive();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Dump Stack", ""))
            {
                LOG_F(INFO, "Help->Dump Stack");
                globals->renderer->DumpJsonToFile("stack.json", globals->renderer->DumpPrimativeStack());
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
    //ImGui::ShowDemoWindow();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(300, globals->renderer->GetWindowSize().y));
    ImGui::Begin("LeftPane", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_MenuBar);
        int hovered_items[2] = {-1, -1};
        if (ImGui::BeginTable("parts_view_table", 1, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("Parts Viewer");
            ImGui::TableHeadersRow();
            for (int row = 0; row < 4; row++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                if (ImGui::TreeNode(std::string("default " + std::to_string(row) + ".dxf").c_str()))
                {
                    for (int dup = 0; dup < 3; dup++)
                    {
                        ImGui::Text("duplicate %d", dup);
                        if (ImGui::IsItemHovered())
                        {
                            hovered_items[0] = row;
                            hovered_items[1] = dup;
                        }
                    }
                    ImGui::TreePop();
                }
                if (ImGui::IsItemHovered())
                {
                    hovered_items[0] = row;
                }
            }
            ImGui::EndTable();
            //ImGui::Text("Hovered master part: %d, duplicate: %d", hovered_items[0], hovered_items[1]);
            if (hovered_items[0] != -1 || hovered_items[1] != -1)
            {
                last_hovered_items[0] = hovered_items[0];
                last_hovered_items[1] = hovered_items[1];
            }
        }
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("New Part"))
            {
            }
            if (last_hovered_items[0] != -1 && last_hovered_items[1] == -1) //We are hovered over a master part
            {
                if (ImGui::MenuItem("Scale Part"))
                {

                }
                if (ImGui::MenuItem("New Duplicate"))
                {

                }
                ImGui::Separator();
                if (ImGui::MenuItem("Show All Parts"))
                {

                }
                if (ImGui::MenuItem("Hide All Parts"))
                {

                }
                if (ImGui::MenuItem("Delete All Parts"))
                {

                }
            }
            if (last_hovered_items[0] != -1 && last_hovered_items[1] != -1) //We are hovered over a master part
            {
                ImGui::Separator();
                if (ImGui::MenuItem("Delete Duplicate Part"))
                {

                }
            }
            ImGui::EndPopup();
        }
    ImGui::End();
}
void jetCamView::PreInit()
{
    this->preferences.background_color[0] = 4 / 255.0f;
    this->preferences.background_color[1] = 17 / 255.0f;
    this->preferences.background_color[2] = 60 / 255.0f;
}
void jetCamView::Init()
{
    globals->renderer->SetCurrentView("jetCamView");

    globals->renderer->PushEvent("up", "scroll", &this->ZoomEventCallback);
    globals->renderer->PushEvent("down", "scroll", &this->ZoomEventCallback);
    this->menu_bar = globals->renderer->PushGui(true, &this->RenderUI);

    this->material_plane = globals->renderer->PushPrimative(new EasyPrimative::Box({0, 0}, globals->nc_control_view->machine_parameters.machine_extents[0], globals->nc_control_view->machine_parameters.machine_extents[1], 0));
    this->material_plane->properties->id = "material_plane";
    this->material_plane->properties->zindex = -20;
    this->material_plane->properties->color[0] = 100;
    this->material_plane->properties->color[1] = 0;
    this->material_plane->properties->color[2] = 0;
    this->material_plane->properties->matrix_callback = &this->ViewMatrixCallback;
    this->material_plane->properties->mouse_callback = &this->MouseEventCallback;
}
void jetCamView::Tick()
{
    
}
void jetCamView::MakeActive()
{
    globals->renderer->SetCurrentView("jetCamView");
    globals->renderer->SetClearColor(this->preferences.background_color[0] * 255.0f, this->preferences.background_color[1] * 255.0f, this->preferences.background_color[2] * 255.0f);
}
void jetCamView::Close()
{
    
}