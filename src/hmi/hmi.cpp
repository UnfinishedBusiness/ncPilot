#include <Xrender.h>
#include <application.h>
#include <vector>
#include <hmi/hmi.h>
#include <iostream>
#include <stdio.h>

double hmi_backplane_width = 300;
Xrender_object_t *hmi_backpane;
double hmi_dro_backplane_height = 200;
Xrender_object_t *hmi_dro_backpane;
Xrender_object_t *hmi_button_backpane;
dro_group_data_t dro;
std::vector<hmi_button_group_t> button_groups;

void mouse_callback(Xrender_object_t* o,nlohmann::json e)
{
    if (o->data["type"] == "box")
    {
        if (e["event"] == "mouse_in")
        {
            //printf("Mouse In - %s\n", e.dump().c_str());
            o->data["color"] = {{"r", 10}, {"g", 100}, {"b", 10}, {"a", 255}};
        }
        if (e["event"] == "mouse_out")
        {
            //printf("Mouse Out - %s\n", e.dump().c_str());
            o->data["color"] = {{"r", 10}, {"g", 10}, {"b", 10}, {"a", 255}};
        }
        if (e["event"] == "left_click_down")
        {
            //printf("Mouse Down - %s\n", e.dump().c_str());
            o->data["color"] = {{"r", 10}, {"g", 150}, {"b", 10}, {"a", 255}};
        }
        if (e["event"] == "left_click_up")
        {
            //printf("Mouse Up - %s\n", e.dump().c_str());
            o->data["color"] = {{"r", 10}, {"g", 100}, {"b", 10}, {"a", 255}};
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
    
    return true;
}
void hmi_resize(nlohmann::json e)
{
    globals->Xcore->data["window_width"] = (int)e["size"]["width"];
    globals->Xcore->data["window_height"] = (int)e["size"]["height"];
    hmi_backpane->data["tl"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2) - hmi_backplane_width}, {"y", -((double)globals->Xcore->data["window_height"] / 2)}};
    hmi_backpane->data["br"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2)}, {"y", ((double)globals->Xcore->data["window_height"] / 2)}};

    hmi_dro_backpane->data["tl"] = {{"x", (((double)globals->Xcore->data["window_width"] / 2) - hmi_backplane_width) + 5}, {"y", ((double)globals->Xcore->data["window_height"] / 2) - 25}};
    hmi_dro_backpane->data["br"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2) - 5}, {"y", (((double)globals->Xcore->data["window_height"] / 2) - 25 - hmi_dro_backplane_height)}};

    double dro_group_x = (double)hmi_dro_backpane->data["tl"]["x"];
    double dro_group_y = (double)hmi_dro_backpane->data["tl"]["y"];
    dro_group_y -= 15;
    dro.x.label->data["position"] = {{"x", dro_group_x + 5}, {"y", dro_group_y - (double)dro.x.label->data["size"]["height"]}};
    dro.x.work_readout->data["position"] = {{"x", dro_group_x + 5 + 50}, {"y", dro_group_y - (double)dro.x.label->data["size"]["height"]}};
    dro.x.absolute_readout->data["position"] = {{"x", dro_group_x + 5 + 220}, {"y", dro_group_y - (double)dro.x.label->data["size"]["height"]}};
    dro.x.divider->data["tl"] = {{"x", dro_group_x + 5}, {"y", dro_group_y - 40}};
    dro.x.divider->data["br"] = {{"x", (double)hmi_dro_backpane->data["br"]["x"] - 5}, {"y", (double)dro.x.divider->data["tl"]["y"] - 8}};
    dro_group_y -= 55;
    dro.y.label->data["position"] = {{"x", dro_group_x + 5}, {"y", dro_group_y - (double)dro.y.label->data["size"]["height"]}};
    dro.y.work_readout->data["position"] = {{"x", dro_group_x + 5 + 50}, {"y", dro_group_y - (double)dro.y.label->data["size"]["height"]}};
    dro.y.absolute_readout->data["position"] = {{"x", dro_group_x + 5 + 220}, {"y", dro_group_y - (double)dro.y.label->data["size"]["height"]}};
    dro.y.divider->data["tl"] = {{"x", dro_group_x + 5}, {"y", dro_group_y - 40}};
    dro.y.divider->data["br"] = {{"x", (double)hmi_dro_backpane->data["br"]["x"] - 5}, {"y", (double)dro.y.divider->data["tl"]["y"] - 8}};
    dro_group_y -= 55;
    dro.z.label->data["position"] = {{"x", dro_group_x + 5}, {"y", dro_group_y - (double)dro.z.label->data["size"]["height"]}};
    dro.z.work_readout->data["position"] = {{"x", dro_group_x + 5 + 50}, {"y", dro_group_y - (double)dro.z.label->data["size"]["height"]}};
    dro.z.absolute_readout->data["position"] = {{"x", dro_group_x + 5 + 220}, {"y", dro_group_y - (double)dro.z.label->data["size"]["height"]}};
    dro.z.divider->data["tl"] = {{"x", dro_group_x + 5}, {"y", dro_group_y - 40}};
    dro.z.divider->data["br"] = {{"x", (double)hmi_dro_backpane->data["br"]["x"] - 5}, {"y", (double)dro.z.divider->data["tl"]["y"] - 8}};
    dro_group_y -= 55;
    dro.feed->data["position"] = {{"x", dro_group_x + 5}, {"y", dro_group_y - (double)dro.feed->data["size"]["height"]}};
    dro.arc_readout->data["position"] = {{"x", dro_group_x + 80}, {"y", dro_group_y - (double)dro.arc_readout->data["size"]["height"]}};
    dro.arc_set->data["position"] = {{"x", dro_group_x + 150}, {"y", dro_group_y - (double)dro.arc_set->data["size"]["height"]}};
    dro.run_time->data["position"] = {{"x", dro_group_x + 210}, {"y", dro_group_y - (double)dro.run_time->data["size"]["height"]}};


    hmi_button_backpane->data["tl"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2) - hmi_backplane_width + 5}, {"y", (((double)globals->Xcore->data["window_height"] / 2) - 25 - hmi_dro_backplane_height - 10)}};
    hmi_button_backpane->data["br"] = {{"x", ((double)globals->Xcore->data["window_width"] / 2) - 5}, {"y", -((double)globals->Xcore->data["window_height"] / 2) + 5}};

    double button_group_x = (double)hmi_button_backpane->data["tl"]["x"];
    double button_group_y = (double)hmi_button_backpane->data["tl"]["y"];
    double button_height = (button_group_y - (double)hmi_button_backpane->data["br"]["y"]) / (double)button_groups.size();
    double button_width = ((double)hmi_button_backpane->data["br"]["x"] - (double)hmi_button_backpane->data["tl"]["x"]) / 2;
    
    //printf("button_group_x: %.4f, button_group_y: %.4f, button_width: %.4f, button_height: %.4f\n", button_group_x, button_group_y, button_width, button_height);
    double center_x;
    double center_y;
    for (int x = 0; x < button_groups.size(); x++)
    {
        button_groups[x].button_one.object->data["tl"] = {{"x", button_group_x + 5}, {"y", button_group_y - 2.5}};
        button_groups[x].button_one.object->data["br"] = {{"x", (button_group_x - 5) + button_width}, {"y", (button_group_y + 2.5) - button_height}};
        center_x = (double)button_groups[x].button_one.object->data["tl"]["x"] + (button_width / 2);
        center_y = (double)button_groups[x].button_one.object->data["tl"]["y"] - (button_height / 2);
        button_groups[x].button_one.label->data["position"] = {{"x", center_x - ((double)button_groups[x].button_one.label->data["size"]["width"] / 2.0f) - 5}, {"y", center_y - ((double)button_groups[x].button_one.label->data["size"]["height"] / 2.0f) + 5}};

        button_group_x += button_width;
        button_groups[x].button_two.object->data["tl"] = {{"x", button_group_x + 5}, {"y", button_group_y - 2.5}};
        button_groups[x].button_two.object->data["br"] = {{"x", (button_group_x - 5) + button_width}, {"y", (button_group_y + 2.5) - button_height}};
        center_x = (double)button_groups[x].button_two.object->data["tl"]["x"] + (button_width / 2);
        center_y = (double)button_groups[x].button_two.object->data["tl"]["y"] - (button_height / 2);
        button_groups[x].button_two.label->data["position"] = {{"x", center_x - ((double)button_groups[x].button_two.label->data["size"]["width"] / 2.0f) - 5}, {"y", center_y - ((double)button_groups[x].button_two.label->data["size"]["height"] / 2.0f) + 5}};
        button_group_x -= button_width;

        button_group_y -= button_height;
    }
}
void hmi_push_button_group(std::string b1, std::string b2)
{
    hmi_button_group_t group;
    group.button_one.name = b1;
    group.button_one.object = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 200},{"color", {{"r", 10},{"g", 10},{"b", 10},{"a", 255}}},});
    group.button_one.label = Xrender_push_text({{"textval", group.button_one.name},{"font", "default"},{"position", {{"x", -10000},{"y", -10000}}},{"font_size", 20},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    group.button_two.name = b2;
    group.button_two.object = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 200},{"color", {{"r", 10},{"g", 10},{"b", 10},{"a", 255}}},});
    group.button_two.label = Xrender_push_text({{"textval", group.button_two.name},{"font", "default"},{"position", {{"x", -10000},{"y", -10000}}},{"font_size", 20},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    group.button_one.object->mouse_callback = mouse_callback;
    group.button_two.object->mouse_callback = mouse_callback;
    button_groups.push_back(group);
}
void hmi_init()
{
    globals->machine_plane = Xrender_push_box({{"tl", {{"x", 0},{"y", 45}}},{"br", {{"x", 45},{"y", 0}}},{"radius", 0},{"zindex", -20},{"color", {{"r", globals->preferences.machine_plane_color[0] * 255},{"g", globals->preferences.machine_plane_color[1] * 255},{"b", globals->preferences.machine_plane_color[2] * 255},{"a", 255}}},});
    globals->machine_plane->matrix_data = view_matrix;
    globals->cuttable_plane = Xrender_push_box({{"tl", {{"x", 1},{"y", 41}}},{"br", {{"x", 41},{"y", 1}}},{"radius", 0},{"zindex", -10},{"color", {{"r", globals->preferences.cuttable_plane_color[0] * 255},{"g", globals->preferences.cuttable_plane_color[1] * 255},{"b", globals->preferences.cuttable_plane_color[2] * 255},{"a", 255}}},});
    globals->cuttable_plane->matrix_data = view_matrix;
    hmi_backpane = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 0},{"zindex", 100},{"color", {{"r", 25},{"g", 44},{"b", 71},{"a", 255}}},});
    hmi_dro_backpane = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 100},{"color", {{"r", 29},{"g", 32},{"b", 48},{"a", 255}}},});
    hmi_button_backpane = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 100},{"color", {{"r", 29},{"g", 32},{"b", 48},{"a", 255}}},});

    hmi_push_button_group("Zero X", "Zero Y");
    hmi_push_button_group("Touch", "Retract");
    hmi_push_button_group("Touch", "Retract");
    hmi_push_button_group("Fit", "Clean");
    hmi_push_button_group("Wpos", "Park");
    hmi_push_button_group("Test Run", "ATHC");
    hmi_push_button_group("Run", "Abort");
 
    dro.x.label = Xrender_push_text({{"textval", "X"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    dro.x.work_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 10},{"g", 150},{"b", 10},{"a", 255},}},});
    dro.x.absolute_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 15},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.x.divider = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 3},{"zindex", 100},{"color", {{"r", 0},{"g", 0},{"b", 0},{"a", 255}}},});

    dro.y.label = Xrender_push_text({{"textval", "Y"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    dro.y.work_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 10},{"g", 150},{"b", 10},{"a", 255},}},});
    dro.y.absolute_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 15},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.y.divider = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 3},{"zindex", 100},{"color", {{"r", 0},{"g", 0},{"b", 0},{"a", 255}}},});

    dro.z.label = Xrender_push_text({{"textval", "Z"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    dro.z.work_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 10},{"g", 150},{"b", 10},{"a", 255},}},});
    dro.z.absolute_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 15},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.z.divider = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 3},{"zindex", 100},{"color", {{"r", 0},{"g", 0},{"b", 0},{"a", 255}}},});

    dro.feed = Xrender_push_text({{"textval", "FEED: 0.0"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.arc_readout = Xrender_push_text({{"textval", "ARC: 0.0"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.arc_set = Xrender_push_text({{"textval", "SET: 0.0"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.run_time = Xrender_push_text({{"textval", "RUN: 00:00:00"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});

    Xrender_push_key_event({"none", "window_resize", hmi_resize});
    Xrender_push_timer(100, hmi_update_timer);
}