#include "application.h"
#include "EasyRender/EasyRender.h"
#include "EasyRender/logging/loguru.h"
#include "EasyRender/gui/imgui.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

EasyRender *renderer;
global_variables_t *globals;

Text *fps;

float zoom = 1;

//Xrender_object_t *performance_label;
//std::vector<int> performance_average;

/*bool debug_render_fps_timer()
{
    performance_average.push_back((int)(1000.0f / (float)Xrender_get_performance()));
    if (performance_average.size() > 10)
    {
        float avg = 0;
        for (int x = 0; x < performance_average.size(); x++) avg += performance_average[x];
        performance_label->data["textval"] = to_string((int)(avg / 10.0f)) + " FPS";
        performance_label->data["position"]["x"] = -((float)globals->Xcore->data["window_width"] / 2.0f) + 10;
        performance_label->data["position"]["y"] = -((float)globals->Xcore->data["window_height"] / 2.0f) + 10;
        performance_average.erase(performance_average.begin());
    }
    return true;
}*/

//globals = new global_variables_t;
    //globals->zoom = 5;
    //globals->pan.x = 0;
    //globals->pan.y = 0;
    //globals->mouse_pos_in_screen_coordinates = {0, 0};
    //globals->mouse_pos_in_matrix_coordinates = {0, 0};
    //globals->start_timestamp = Xrender_millis();

void mouse_callback(PrimativeContainer* p, nlohmann::json e)
{
    LOG_F(INFO, "%s", e.dump().c_str());
    if (e["type"] == "mouse_in")
    {
        renderer->SetColorByName(p->properties->color, "green");
    }
    if (e["type"] == "mouse_out")
    {
        renderer->SetColorByName(p->properties->color, "white");
    }
}
void view_matrix(PrimativeContainer *p)
{
    p->properties->scale = zoom;
}
bool fps_timer()
{
    fps->textval = std::to_string(renderer->GetFramesPerSecond());
    return true;
}
EasyRender::EasyRenderGui *w;
void test_dialog()
{
    ImGui::Begin("Question?", &w->visable, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::Button("Yes"))
    {
        
    }
    ImGui::SameLine();
    if (ImGui::Button("No"))
    {
        
    }
    ImGui::End();
}
void zoom_handle(nlohmann::json e)
{
    LOG_F(INFO, "%s", e.dump().c_str());
    if ((float)e["scroll"] > 0)
    {
        zoom += zoom * 0.1;
    }
    else
    {
        zoom -= zoom * 0.1;
    }
}
int main(int argc, char **argv)
{
    renderer = new EasyRender();

    renderer->Init(argc, argv);

    Line *l = renderer->PushPrimative(new Line({0, 0}, {100, 100}));
    l->properties->scale = 0.5;
    l->properties->offset[0] = 50;
    l->properties->offset[1] = -50;
    l->properties->mouse_callback = &mouse_callback;
    l->properties->matrix_callback = &view_matrix;

    fps = renderer->PushPrimative(new Text({0, 0}, "0", 30));
    fps->properties->mouse_callback = &mouse_callback;

    std::vector<double_point_t> path;
    path.push_back({20, 20});
    path.push_back({40, 40});
    path.push_back({80, 50});
    Path *p = renderer->PushPrimative(new Path(path));
    p->properties->mouse_callback = &mouse_callback;
    p->properties->matrix_callback = &view_matrix;

    Arc *a = renderer->PushPrimative(new Arc({-100, -100}, 100, 0, 90));
    a->properties->mouse_callback = &mouse_callback;
    a->properties->matrix_callback = &view_matrix;

    Circle *c = renderer->PushPrimative(new Circle({-200, -200}, 100));
    c->properties->mouse_callback = &mouse_callback;
    c->properties->matrix_callback = &view_matrix;

    Box *b = renderer->PushPrimative(new Box({100, -100}, 100, 100, 30));
    b->properties->mouse_callback = &mouse_callback;

    renderer->PushTimer(1000, &fps_timer);

    w = renderer->PushGui(true, &test_dialog);

    renderer->PushEvent("up", "scroll", &zoom_handle);
    renderer->PushEvent("down", "scroll", &zoom_handle);

    while(renderer->Poll(false))
    {
        //Do stuff
    }

    renderer->Close();
    delete renderer;
    return 0;
}