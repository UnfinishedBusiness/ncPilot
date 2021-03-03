#include "jetCamView.h"
#include <EasyRender/EasyRender.h>
#include <EasyRender/logging/loguru.h>
#include <EasyRender/gui/imgui.h>
#include <EasyRender/gui/ImGuiFileDialog.h>
#include <dxf/dxflib/dl_dxf.h>
#include "DXFParseAdaptor/DXFParseAdaptor.h"

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
void jetCamView::MouseEventCallback(PrimativeContainer* c,nlohmann::json e)
{
    //LOG_F(INFO, "%s", e.dump().c_str());
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
                if (ImGui::MenuItem("Import Part", ""))
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
                for (int i = 0; i < self->parts_stack.size(); i++)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Checkbox(std::string("##" + self->parts_stack[i]->filename).c_str(), &self->parts_stack[i]->visable);
                    ImGui::SameLine();
                    if (ImGui::TreeNode(self->parts_stack[i]->filename.c_str()))
                    {
                        for (int dup = 0; dup < 3; dup++)
                        {
                            ImGui::Text("duplicate %d", dup);
                            if (ImGui::IsItemHovered())
                            {
                                hovered_items[0] = i;
                                hovered_items[1] = dup;
                            }
                        }
                        ImGui::TreePop();
                    }
                    if (ImGui::IsItemHovered())
                    {
                        hovered_items[0] = i;
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
bool jetCamView::DxfFileOpen(std::string filename, std::string name)
{
    this->dxf_fp = fopen(filename.c_str(), "rt");
    if (this->dxf_fp)
    {
        jetCamView::part_viewer_t *part = new jetCamView::part_viewer_t;
        part->visable = false;
        part->last_visable = part->visable;
        part->filename = name;
        this->parts_stack.push_back(part);
        this->dl_dxf = new DL_Dxf();
        this->DXFcreationInterface = new DXFParseAdaptor(globals->renderer, &this->ViewMatrixCallback, &this->MouseEventCallback);
        this->DXFcreationInterface->SetFilename(part->filename);
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
                for (int i = 0; i < self->parts_stack.size(); i++)
                {
                    if (self->parts_stack.at(i)->filename == self->DXFcreationInterface->filename)
                    {
                        self->SetPartVisable(i, true);
                    }
                }
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
void jetCamView::SetPartVisable(int i, bool v)
{
    if (this->parts_stack.size()+1 > i)
    {
        //LOG_F(INFO, "Setting part: %d to %d", i, (int)v);
        this->parts_stack.at(i)->visable = v;
        this->parts_stack.at(i)->last_visable = this->parts_stack.at(i)->visable;
        for (std::vector<PrimativeContainer*>::iterator it = globals->renderer->GetPrimativeStack()->begin(); it != globals->renderer->GetPrimativeStack()->end(); ++it)
        {
            if ((*it)->properties->view == globals->renderer->GetCurrentView())
            {
                if ((*it)->properties->data["filename"] == this->parts_stack[i]->filename)
                {
                    (*it)->properties->visable = v;
                }
            }
        }
    }
}
void jetCamView::PreInit()
{
    this->preferences.background_color[0] = 4 / 255.0f;
    this->preferences.background_color[1] = 17 / 255.0;
    this->preferences.background_color[2] = 60 / 255.0f;
}
void jetCamView::Init()
{
    globals->renderer->SetCurrentView("jetCamView");

    globals->renderer->PushEvent("up", "scroll", &this->ZoomEventCallback);
    globals->renderer->PushEvent("down", "scroll", &this->ZoomEventCallback);
    this->menu_bar = globals->renderer->PushGui(true, &this->RenderUI, this);

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
    for (int i = 0; i < this->parts_stack.size(); i++)
    {
        if (this->parts_stack.at(i)->last_visable != this->parts_stack.at(i)->visable)
        {
            this->SetPartVisable(i, this->parts_stack[i]->visable);
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
    for (int i = 0; i < this->parts_stack.size(); i++)
    {
        delete this->parts_stack.at(i);
        this->parts_stack.erase(this->parts_stack.begin()+i);
    }
}