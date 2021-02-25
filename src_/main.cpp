#include "application.h"
#include "EasyRender/EasyRender.h"
#include "EasyRender/logging/loguru.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

EasyRender *renderer;
global_variables_t *globals;

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
}

int main(int argc, char **argv)
{
    renderer = new EasyRender();

    renderer->Init(argc, argv);

    Line *l = renderer->PushPrimative(new Line({0, 0}, {100, 100}));
    l->properties->scale = 0.5;
    l->properties->offset[0] = 50;
    l->properties->offset[1] = -50;

    while(renderer->Poll(false))
    {
        //Do stuff
    }

    delete renderer;
    return 0;
}