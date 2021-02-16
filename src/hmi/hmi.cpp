#include <Xrender.h>
#include <application.h>
#include <vector>
#include <hmi/hmi.h>
#include <iostream>
#include <stdio.h>

double hmi_backplane_width = 300;
Xrender_object_t *hmi_backpane;
Xrender_object_t *hmi_dro_backpane;

void mouse_callback(Xrender_object_t* o,nlohmann::json e)
{
    if (o->data["type"] == "box")
    {
        if (e["event"] == "mouse_in")
        {
            
        }
        if (e["event"] == "mouse_out")
        {
            
        }
        if (e["event"] == "left_click_down")
        {
            
        }
    }
}

nlohmann::json view_matrix(nlohmann::json data)
{
    nlohmann::json new_data = data;
    if (data["type"] == "line")
    {
        new_data["start"]["x"] = ((double)data["start"]["x"] * globals->zoom) + globals->pan.x;
        new_data["start"]["y"] = ((double)data["start"]["y"] * globals->zoom) + globals->pan.y;
        new_data["end"]["x"] = ((double)data["end"]["x"] * globals->zoom) + globals->pan.x;
        new_data["end"]["y"] = ((double)data["end"]["y"] * globals->zoom) + globals->pan.y;
    }
    if (data["type"] == "arc" || data["type"] == "circle")
    {
        new_data["center"]["x"] = ((double)data["center"]["x"] * globals->zoom) + globals->pan.x;
        new_data["center"]["y"] = ((double)data["center"]["y"]* globals->zoom) + globals->pan.y;
        new_data["radius"] = ((double)data["radius"] * globals->zoom);
    }
    if (data["type"] == "box")
    {
        new_data["tl"]["x"] = ((double)data["tl"]["x"] * globals->zoom) + globals->pan.x;
        new_data["tl"]["y"] = ((double)data["tl"]["y"] * globals->zoom) + globals->pan.y;
        new_data["br"]["x"] = ((double)data["br"]["x"] * globals->zoom) + globals->pan.x;
        new_data["br"]["y"] = ((double)data["br"]["y"] * globals->zoom) + globals->pan.y;
    }
    return new_data;
}
bool hmi_update_timer()
{
    hmi_backpane->data["tl"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2) - hmi_backplane_width}, {"y", -((double)globals->Xcore->data["window_height"] / 2)}};
    hmi_backpane->data["br"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2)}, {"y", ((double)globals->Xcore->data["window_height"] / 2)}};

    hmi_dro_backpane->data["tl"] = {{"x", (((double)globals->Xcore->data["window_width"] / 2) - hmi_backplane_width) + 5}, {"y", ((double)globals->Xcore->data["window_height"] / 2) - 25}};
    hmi_dro_backpane->data["br"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2) - 5}, {"y", (((double)globals->Xcore->data["window_height"] / 2) - 25 - 200)}};
    return true;
}
void hmi_init()
{
    globals->machine_plane = Xrender_push_box({
        {"tl", {
            {"x", 0},
            {"y", 45}
        }},
        {"br", {
            {"x", 45},
            {"y", 0}
        }},
        {"radius", 0},
        {"zindex", -20},
        {"color", {
            {"r", globals->preferences.machine_plane_color[0] * 255},
            {"g", globals->preferences.machine_plane_color[1] * 255},
            {"b", globals->preferences.machine_plane_color[2] * 255},
            {"a", 255}
        }},
    });
    globals->machine_plane->matrix_data = view_matrix;

    globals->cuttable_plane = Xrender_push_box({
        {"tl", {
            {"x", 1},
            {"y", 41}
        }},
        {"br", {
            {"x", 41},
            {"y", 1}
        }},
        {"radius", 0},
        {"zindex", -10},
        {"color", {
            {"r", globals->preferences.cuttable_plane_color[0] * 255},
            {"g", globals->preferences.cuttable_plane_color[1] * 255},
            {"b", globals->preferences.cuttable_plane_color[2] * 255},
            {"a", 255}
        }},
    });
    globals->cuttable_plane->matrix_data = view_matrix;

    hmi_backpane = Xrender_push_box({
        {"tl", {
            {"x", -100000},
            {"y", -100000}
        }},
        {"br", {
            {"x", -100000},
            {"y", -100000}
        }},
        {"radius", 0},
        {"zindex", 100},
        {"color", {
            {"r", 25},
            {"g", 44},
            {"b", 71},
            {"a", 255}
        }},
    });
    hmi_dro_backpane = Xrender_push_box({
        {"tl", {
            {"x", -100000},
            {"y", -100000}
        }},
        {"br", {
            {"x", -100000},
            {"y", -100000}
        }},
        {"radius", 5},
        {"zindex", 100},
        {"color", {
            {"r", 29},
            {"g", 32},
            {"b", 48},
            {"a", 255}
        }},
    });
    Xrender_push_timer(100, hmi_update_timer);
}