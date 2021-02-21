#include <Xrender.h>
#include <application.h>
#include <vector>
#include <stdio.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include "logging/loguru.h"

using namespace std;

Xrender_object_t *performance_label;
std::vector<int> performance_average;

bool debug_render_fps_timer()
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
}
void debug_dump_stack()
{
    LOG_F(INFO, "Dumping Xrender stack!");
    std::vector<Xrender_object_t *> *stack = Xrender_get_object_stack();
    std::ofstream out("object_stack.dump");
    for (int x = 0; x < stack->size(); x++)
    {
        out << std::to_string(x) << "> " << std::string(stack->at(x)->data.dump()) << std::endl;
    }
    out.close();
    LOG_F(INFO, "Finished Dumping!");
}
void debug_init()
{
    performance_label = Xrender_push_text({
        {"textval", "0"},
        {"font", "default"},
        {"position", {
            {"x", -10000},
            {"y", -10000}
        }},
        {"font_size", 20},
        {"angle", 0},
        {"color", {
            {"r", 255},
            {"g", 255},
            {"b", 255},
            {"a", 255},
        }},
    });
    Xrender_push_timer(100, debug_render_fps_timer);
}