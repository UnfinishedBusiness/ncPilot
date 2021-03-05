#include "jetCamView.h"
#include <EasyRender/EasyRender.h>
#include <EasyRender/logging/loguru.h>
#include <EasyRender/gui/imgui.h>
#include <EasyRender/gui/ImGuiFileDialog.h>
#include <dxf/dxflib/dl_dxf.h>
#include "DXFParsePathAdaptor/DXFParsePathAdaptor.h"

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
    p->properties->scale = globals->zoom;
    p->properties->offset[0] = globals->pan.x;
    p->properties->offset[1] = globals->pan.y;
}
void jetCamView::MouseCallback(nlohmann::json e)
{
    //LOG_F(INFO, "%s", e.dump().c_str());
    if (!globals->renderer->imgui_io->WantCaptureKeyboard || !globals->renderer->imgui_io->WantCaptureMouse)
    {
        if (e["event"] == "left_click_up")
        {
            globals->jet_cam_view->left_click_state = false;
        }
        if (e["event"] == "left_click_down")
        {
            globals->jet_cam_view->left_click_state = true;
        }
        if (e["event"] == "mouse_move")
        {
            float light_green[4];
            globals->renderer->SetColorByName(light_green, "light-green");
            double_point_t mouse_drag = { (double)e["pos"]["x"] - globals->jet_cam_view->last_mouse_click_position.x, (double)e["pos"]["y"] - globals->jet_cam_view->last_mouse_click_position.y };
            if (globals->jet_cam_view->CurrentTool == JETCAM_TOOL_NESTING)
            {
                if (globals->jet_cam_view->left_click_state == true)
                {
                    //LOG_F(INFO, "Dragging mouse!");
                    for (std::vector<PrimativeContainer*>::iterator it = globals->renderer->GetPrimativeStack()->begin(); it != globals->renderer->GetPrimativeStack()->end(); ++it)
                    {
                        if ((*it)->properties->view == globals->renderer->GetCurrentView())
                        {
                            if ((*it)->type == "part")
                            {
                                if ((*it)->part->is_part_selected == true)
                                {
                                    //LOG_F(INFO, "Moving (%.4f, %.4f)", mouse_drag.x, mouse_drag.y);
                                    (*it)->part->control.offset.x += (mouse_drag.x / globals->zoom);
                                    (*it)->part->control.offset.y += (mouse_drag.y / globals->zoom);
                                }
                            }
                        }
                    }
                }   
            }
            globals->jet_cam_view->last_mouse_click_position = { (double)e["pos"]["x"], (double)e["pos"]["y"] };
        }
    }
    else
    {
        globals->jet_cam_view->left_click_state = false;
    }
}
void jetCamView::MouseEventCallback(PrimativeContainer* c,nlohmann::json e)
{
    //LOG_F(INFO, "%s", e.dump().c_str());
    if (globals->jet_cam_view->CurrentTool == JETCAM_TOOL_CONTOUR)
    {
        if (c->type == "part")
        {
            if (e["event"] == "mouse_in")
            {
                size_t x = (size_t)e["path_index"];
                globals->renderer->SetColorByName(c->part->paths[x].color, "light-green");
            }
            if (e["event"] == "mouse_out")
            {
                for (size_t x = 0; x < c->part->paths.size(); x++)
                {
                    if (c->part->paths[x].is_inside_contour == true)
                    {
                        globals->renderer->SetColorByName(c->part->paths[x].color, "grey");
                    }
                    else
                    {
                        globals->renderer->SetColorByName(c->part->paths[x].color, "white");
                    }
                }
            }
        }
    }
    if (globals->jet_cam_view->CurrentTool == JETCAM_TOOL_NESTING)
    {
        if (c->type == "part" && globals->jet_cam_view->left_click_state == false)
        {
            if (e["event"] == "mouse_in")
            {
                c->part->is_part_selected = true;
                for (size_t x = 0; x < c->part->paths.size(); x++)
                {
                    globals->renderer->SetColorByName(c->part->paths[x].color, "light-green");
                }
            }
            if (e["event"] == "mouse_out")
            {
                for (size_t x = 0; x < c->part->paths.size(); x++)
                {
                    if (c->part->paths[x].is_inside_contour == true)
                    {
                        globals->renderer->SetColorByName(c->part->paths[x].color, "grey");
                    }
                    else
                    {
                        globals->renderer->SetColorByName(c->part->paths[x].color, "white");
                    }
                }
                c->part->is_part_selected = false;
            }
        }
    }
}
int last_hovered_items[2] = {-1, -1};
void jetCamView::RenderUI(void *self_pointer)
{
    jetCamView *self = reinterpret_cast<jetCamView *>(self_pointer);
    if (self != NULL)
    {
        if (ImGuiFileDialog::Instance()->Display("ImportPartDialog", ImGuiWindowFlags_NoCollapse, ImVec2(600, 500)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                std::string fileName = ImGuiFileDialog::Instance()->GetCurrentFileName();
                LOG_F(INFO, "File Path: %s, File Path Name: %s, File Name: %s", filePath.c_str(), filePathName.c_str(), fileName.c_str());
                globals->renderer->StringToFile(globals->renderer->GetConfigDirectory() + "last_dxf_open_path.conf", filePath + "/");
                self->DxfFileOpen(filePathName, fileName);
            }
            ImGuiFileDialog::Instance()->Close();
        }
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Part", ""))
                {
                    LOG_F(INFO, "File->Open");
                    std::string path = ".";
                    std::string p = globals->renderer->FileToString(globals->renderer->GetConfigDirectory() + "last_dxf_open_path.conf");
                    if (p != "")
                    {
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
            ImGui::RadioButton("Contour Tool", &self->CurrentTool, 0); ImGui::SameLine();
            ImGui::RadioButton("Nesting Tool", &self->CurrentTool, 1); ImGui::SameLine();
            ImGui::RadioButton("Point Tool", &self->CurrentTool, 2);
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
                int count = 0;
                for (std::vector<PrimativeContainer*>::iterator it = globals->renderer->GetPrimativeStack()->begin(); it != globals->renderer->GetPrimativeStack()->end(); ++it)
                {
                    if ((*it)->properties->view == globals->renderer->GetCurrentView() && (*it)->type == "part")
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Checkbox(std::string("##" + (*it)->part->part_name).c_str(), &(*it)->properties->visable);
                        ImGui::SameLine();
                        if (ImGui::TreeNode((*it)->part->part_name.c_str()))
                        {
                            /*for (int dup = 0; dup < self->parts_stack[i]->duplicates.size(); dup++)
                            {
                                ImGui::Text("Duplicate %d", dup);
                                if (ImGui::IsItemHovered())
                                {
                                    hovered_items[0] = i;
                                    hovered_items[1] = dup;
                                }
                            }*/
                            ImGui::TreePop();
                        }
                    }
                    if (ImGui::IsItemHovered())
                    {
                        hovered_items[0] = count;
                    }
                    count++;
                }
                ImGui::EndTable();
                //ImGui::Text("Hovered master part: %d, duplicate: %d", hovered_items[0], hovered_items[1]);
                if (hovered_items[0] != -1 || hovered_items[1] != -1)
                {
                    last_hovered_items[0] = hovered_items[0];
                    last_hovered_items[1] = hovered_items[1];
                }
            }
            if (ImGui::BeginPopupContextWindow("right_click_menu"))
            {
                if (ImGui::MenuItem("New Part"))
                {
                    LOG_F(INFO, "Right Click->New Part");
                    std::string path = ".";
                    std::string p = globals->renderer->FileToString(globals->renderer->GetConfigDirectory() + "last_dxf_open_path.conf");
                    if (p != "")
                    {
                        path = p;
                    }
                    ImGuiFileDialog::Instance()->OpenDialog("ImportPartDialog", "Choose File", ".dxf", path.c_str());
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
                    if (ImGui::MenuItem("Show Duplicate Parts"))
                    {
                    }
                    if (ImGui::MenuItem("Hide Duplicate Parts"))
                    {
                    }
                    if (ImGui::MenuItem("Delete Master Part"))
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
}
void jetCamView::RenderProgressWindow(void *p)
{
    jetCamView *self = reinterpret_cast<jetCamView *>(p);
    if (self != NULL)
    {
        ImGui::Begin("Progress", &self->ProgressWindowHandle->visable, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::ProgressBar(self->ProgressWindowProgress, ImVec2(0.0f, 0.0f));
            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
            ImGui::Text("Progress");
        ImGui::End();
    }
}
void jetCamView::ShowProgressWindow(bool v)
{
    this->ProgressWindowProgress = 0.0;
    this->ProgressWindowHandle->visable = true;
}
bool jetCamView::DxfFileOpen(std::string filename, std::string name)
{
    this->dxf_fp = fopen(filename.c_str(), "rt");
    if (this->dxf_fp)
    {
        this->dl_dxf = new DL_Dxf();
        this->DXFcreationInterface = new DXFParsePathAdaptor(globals->renderer, &this->ViewMatrixCallback, &this->MouseEventCallback);
        this->DXFcreationInterface->SetFilename(name);
        this->DXFcreationInterface->SetScaleFactor(1);
        //this->DXFcreationInterface->SetSmoothing(0.030);
        globals->renderer->PushTimer(0, this->DxfFileParseTimer, this);
        LOG_F(INFO, "Parsing DXF File: %s", filename.c_str());  
        return true;
    }
    return false;
}
bool jetCamView::DxfFileParseTimer(void *p)
{
    jetCamView *self = reinterpret_cast<jetCamView *>(p);
    if (self != NULL)
    {
        for (int x = 0; x < 500; x++)
        {
            if (!self->dl_dxf->readDxfGroups(self->dxf_fp, self->DXFcreationInterface))
            {
                self->DXFcreationInterface->Finish();
                fclose(self->dxf_fp);
                delete self->DXFcreationInterface;
                delete self->dl_dxf;
                return false;
            }
        }
        return true;
    }
    return false;
}
void jetCamView::PreInit()
{
    this->preferences.background_color[0] = 4 / 255.0f;
    this->preferences.background_color[1] = 17 / 255.0;
    this->preferences.background_color[2] = 60 / 255.0f;
    this->CurrentTool = JETCAM_TOOL_CONTOUR;
    this->left_click_state = false;
}
void jetCamView::Init()
{
    globals->renderer->SetCurrentView("jetCamView");
    globals->renderer->PushEvent("up", "scroll", &this->ZoomEventCallback);
    globals->renderer->PushEvent("down", "scroll", &this->ZoomEventCallback);
    globals->renderer->PushEvent("none", "left_click_up", &this->MouseCallback);
    globals->renderer->PushEvent("none", "left_click_down", &this->MouseCallback);
    globals->renderer->PushEvent("none", "right_click_up", &this->MouseCallback);
    globals->renderer->PushEvent("none", "right_click_down", &this->MouseCallback);
    globals->renderer->PushEvent("none", "mouse_move", &this->MouseCallback);
    this->menu_bar = globals->renderer->PushGui(true, &this->RenderUI, this);
    this->ProgressWindowHandle = globals->renderer->PushGui(false, &this->RenderProgressWindow, this);
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
    for (std::vector<PrimativeContainer*>::iterator it = globals->renderer->GetPrimativeStack()->begin(); it != globals->renderer->GetPrimativeStack()->end(); ++it)
    {
        if ((*it)->properties->view == globals->renderer->GetCurrentView())
        {
            if ((*it)->type == "part")
            {
                (*it)->part->control.mouse_mode = this->CurrentTool;
            }
        }
    }
}
void jetCamView::MakeActive()
{
    globals->renderer->SetCurrentView("jetCamView");
    globals->renderer->SetClearColor(this->preferences.background_color[0] * 255.0f, this->preferences.background_color[1] * 255.0f, this->preferences.background_color[2] * 255.0f);
}
void jetCamView::Close()
{
    
}