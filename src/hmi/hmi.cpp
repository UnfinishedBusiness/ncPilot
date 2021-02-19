#include <Xrender.h>
#include <application.h>
#include <vector>
#include <hmi/hmi.h>
#include <motion_control/motion_control.h>
#include <gcode/gcode.h>
#include <dialogs/dialogs.h>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
#include <iterator>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "logging/loguru.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

double hmi_backplane_width = 300;
Xrender_object_t *hmi_backpane;
double hmi_dro_backplane_height = 200;
Xrender_object_t *hmi_dro_backpane;
Xrender_object_t *hmi_button_backpane;
dro_group_data_t dro;
Xrender_object_t *way_point_marker;
std::vector<hmi_button_group_t> button_groups;
double_point_t way_point_position = {-1000, -1000};

void hmi_get_bounding_box(double_point_t *bbox_min, double_point_t *bbox_max)
{
    std::vector<Xrender_object_t*> *stack = Xrender_get_object_stack();
    bbox_max->x = -1000000;
    bbox_max->y = -1000000;
    bbox_min->x = 1000000;
    bbox_min->y = 1000000;
    for (int x = 0; x < stack->size(); x++)
    {
        if (stack->at(x)->data["type"] == "path")
        {
            for (int i = 0; i < stack->at(x)->data["points"].size(); i++)
            {
                if ((double)stack->at(x)->data["points"].at(i)["x"] + globals->machine_parameters.work_offset[0] < bbox_min->x) bbox_min->x = (double)stack->at(x)->data["points"].at(i)["x"] + globals->machine_parameters.work_offset[0];
                if ((double)stack->at(x)->data["points"].at(i)["x"] + globals->machine_parameters.work_offset[0] > bbox_max->x) bbox_max->x = (double)stack->at(x)->data["points"].at(i)["x"] + globals->machine_parameters.work_offset[0];
                if ((double)stack->at(x)->data["points"].at(i)["y"] + globals->machine_parameters.work_offset[1] < bbox_min->y) bbox_min->y = (double)stack->at(x)->data["points"].at(i)["y"] + globals->machine_parameters.work_offset[1];
                if ((double)stack->at(x)->data["points"].at(i)["y"] + globals->machine_parameters.work_offset[1] > bbox_max->y) bbox_max->y = (double)stack->at(x)->data["points"].at(i)["y"] + globals->machine_parameters.work_offset[1];
            }
        }
    }
}

void hmi_handle_button(std::string id)
{
    nlohmann::json dro_data = motion_controller_get_dro();
    try
     {
        if (dro_data["IN_MOTION"] == false)
        {
            if (id == "Wpos")
            {
                LOG_F(INFO, "Clicked Wpos");
                motion_controller_push_stack("G0 X10 Y10");
                motion_controller_push_stack("M30");
                motion_controller_run_stack();
            }
            else if (id == "Park")
            {
                LOG_F(INFO, "Clicked Park");
                motion_controller_push_stack("G53 G0 Z0");
                motion_controller_push_stack("G53 G0 X0 Y0");
                motion_controller_push_stack("M30");
                motion_controller_run_stack();
            }
            else if (id == "Zero X")
            {
                LOG_F(INFO, "Clicked Zero X");
                try
                {
                    globals->machine_parameters.work_offset[0] = (float)motion_controller_get_dro()["MCS"]["x"];
                    motion_controller_push_stack("G10 L2 P0 X" + to_string(globals->machine_parameters.work_offset[0]));
                    motion_controller_push_stack("M30");
                    motion_controller_run_stack();
                    motion_controller_save_machine_parameters();
                }
                catch (...)
                {
                    LOG_F(ERROR, "Could not set x work offset!");
                }
            }
            else if (id == "Zero Y")
            {
                LOG_F(INFO, "Clicked Zero Y");
                try
                {
                    globals->machine_parameters.work_offset[1] = (float)motion_controller_get_dro()["MCS"]["y"];
                    motion_controller_push_stack("G10 L2 P0 Y" + to_string(globals->machine_parameters.work_offset[1]));
                    motion_controller_push_stack("M30");
                    motion_controller_run_stack();
                    motion_controller_save_machine_parameters();
                }
                catch (...)
                {
                    LOG_F(ERROR, "Could not set y work offset!");
                }
            }
            else if (id == "Retract")
            {
                LOG_F(INFO, "Clicked Retract");
                motion_controller_push_stack("G53 G0 Z0");
                motion_controller_push_stack("M30");
                motion_controller_run_stack();
            }
            else if (id == "Touch")
            {
                LOG_F(INFO, "Clicked Touch");
                motion_controller_push_stack("touch_torch");
                motion_controller_push_stack("M30");
                motion_controller_run_stack();
            }
            else if (id == "Run")
            {
                LOG_F(INFO, "Clicked Run");
                try
                {
                    if (gcode_get_filename() != "")
                    {
                        std::ifstream gcode_file(gcode_get_filename());
                        if (gcode_file.is_open())
                        {
                            std::string line;
                            while (std::getline(gcode_file, line))
                            {
                                motion_controller_push_stack(line);
                            }
                            gcode_file.close();
                            motion_controller_run_stack();
                        }
                        else
                        {
                            dialogs_set_info_value("Could not open file!");
                        }
                    }
                }
                catch(...)
                {
                    dialogs_set_info_value("Caught exception while trying to read file!");
                }
            }
            else if (id == "Test Run")
            {
                LOG_F(INFO, "Clicked Test Run");
            }
        }
        else
        {
            if (id == "Abort")
            {
                LOG_F(INFO, "Clicked Abort");
                motion_controller_cmd("abort");
            }
        }
        if (id == "Clean")
        {
            LOG_F(INFO, "Clicked Clean");
        }
        else if (id == "Fit")
        {
            LOG_F(INFO, "Clicked Fit");
            double_point_t bbox_min, bbox_max;
            hmi_get_bounding_box(&bbox_min, &bbox_max);
            if (bbox_max.x == -1000000 && bbox_max.y == -1000000 && bbox_min.x == 1000000 && bbox_min.y == 1000000)
            {
                LOG_F(INFO, "No paths, fitting to machine extents!");
                globals->zoom = 1;
                globals->pan.x = ((globals->machine_parameters.machine_extents[0]) / 2) - (hmi_backplane_width / 2);
                globals->pan.y = (((globals->machine_parameters.machine_extents[1]) / 2) + 10) * -1; //10 is for the menu bar
                if ((MAX((double)globals->Xcore->data["window_height"], (double)globals->machine_parameters.machine_extents[1]) - MIN((double)globals->Xcore->data["window_height"], (double)globals->machine_parameters.machine_extents[1])) < 
                    MAX((double)globals->Xcore->data["window_width"] - hmi_backplane_width, (double)globals->machine_parameters.machine_extents[0]) - MIN((double)globals->Xcore->data["window_width"], (double)globals->machine_parameters.machine_extents[0]))
                {
                    globals->zoom = ((double)globals->Xcore->data["window_height"] - 100) / ((double)globals->machine_parameters.machine_extents[1]);
                    globals->pan.x += ((double)globals->machine_parameters.machine_extents[0] / 2) * (1 - globals->zoom);
                    globals->pan.y += ((double)globals->machine_parameters.machine_extents[1] / 2) * (1 - globals->zoom);
                }
                else //Fit X
                {
                    globals->zoom = ((double)globals->Xcore->data["window_width"] - (hmi_backplane_width / 2) - 200) / ((double)globals->machine_parameters.machine_extents[0]);
                    globals->pan.x += ((double)globals->machine_parameters.machine_extents[0] / 2) * (1 - globals->zoom) - (hmi_backplane_width / 2) + 50;
                    globals->pan.y += ((double)globals->machine_parameters.machine_extents[1] / 2) * (1 - globals->zoom);
                }
            }
            else
            {
                LOG_F(INFO, "Calculated bounding box: => bbox_min = (%.4f, %.4f) and bbox_max = (%.4f, %.4f)", bbox_min.x, bbox_min.y, bbox_max.x, bbox_max.y);
                globals->zoom = 1;
                globals->pan.x = ((bbox_max.x - bbox_min.x) / 2) - (hmi_backplane_width / 2);
                globals->pan.y = ((((bbox_max.y - bbox_min.y) / 2) + 10) * -1); //10 is for the menu bar
                if ((MAX((double)globals->Xcore->data["window_height"], (bbox_max.x - bbox_min.x)) - MIN((double)globals->Xcore->data["window_height"], (bbox_max.y - bbox_min.y))) < 
                    MAX((double)globals->Xcore->data["window_width"] - hmi_backplane_width, (bbox_max.x - bbox_min.x)) - MIN((double)globals->Xcore->data["window_width"], (bbox_max.y - bbox_min.y)))
                {
                    LOG_F(INFO, "Fitting to Y");
                    globals->zoom = ((double)globals->Xcore->data["window_height"] - 300) / (bbox_max.y - bbox_min.y);
                    globals->pan.x += (bbox_max.x - bbox_min.x) * (1 - globals->zoom) + (hmi_backplane_width / 2);
                    globals->pan.y += (bbox_max.y - bbox_min.y) * (1 - globals->zoom);
                    globals->pan.x -= globals->machine_parameters.work_offset[0] * globals->zoom;
                    globals->pan.y -= globals->machine_parameters.work_offset[1] * globals->zoom;
                }
                else //Fit X
                {
                    LOG_F(INFO, "Fitting to X");
                    globals->zoom = ((double)globals->Xcore->data["window_width"] - (hmi_backplane_width / 2) - 200) / (bbox_max.x - bbox_min.x);
                    globals->pan.x += (bbox_max.x - bbox_min.x) * (1 - globals->zoom) + (hmi_backplane_width / 2);
                    globals->pan.y += (bbox_max.y - bbox_min.y) * (1 - globals->zoom);
                    globals->pan.x -= globals->machine_parameters.work_offset[0] * globals->zoom;
                    globals->pan.y -= globals->machine_parameters.work_offset[1] * globals->zoom;
                }
            }
        }
        else if (id == "ATHC")
        {
            LOG_F(INFO, "Clicked ATHC");
        }
    }
    catch(...)
    {
        LOG_F(ERROR, "JSON Parsing ERROR!");
    }
}
void mouse_callback(Xrender_object_t* o,nlohmann::json e)
{
    if (o->data["type"] == "box" && o->data["id"] != "cuttable_plane")
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
            hmi_handle_button(o->data["id"]);
        }
    }
    else if (o->data["type"] == "box" && o->data["id"] == "cuttable_plane")
    {
        if (e["event"] == "left_click_up")
        {
            nlohmann::json dro_data = motion_controller_get_dro();
            try
            {
                if (o->data["id"] == "cuttable_plane" && dro_data["IN_MOTION"] == false)
                {
                    way_point_position = globals->mouse_pos_in_matrix_coordinates;
                    way_point_marker->data["center"] = {{"x", way_point_position.x}, {"y", way_point_position.y}};
                    way_point_marker->data["visable"] = true;
                }
            }
            catch(...)
            {
                LOG_F(ERROR, "Error parsing DRO Data!");
            }
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
        if (data["id"] == "torch_pointer" || data["id"] == "way_point_marker")
        {
            new_data["center"]["x"] = ((double)data["center"]["x"] * globals->zoom) + globals->pan.x;
            new_data["center"]["y"] = ((double)data["center"]["y"]* globals->zoom) + globals->pan.y;
            //new_data["radius"] = ((double)data["radius"] * globals->zoom);
        }
        else
        {
            new_data["center"]["x"] = ((double)data["center"]["x"] * globals->zoom) + globals->pan.x;
            new_data["center"]["y"] = ((double)data["center"]["y"]* globals->zoom) + globals->pan.y;
            new_data["radius"] = ((double)data["radius"] * globals->zoom);
        }
    }
    if (data["type"] == "box")
    {
        new_data["tl"]["x"] = ((double)data["tl"]["x"] * globals->zoom) + globals->pan.x;
        new_data["tl"]["y"] = ((double)data["tl"]["y"] * globals->zoom) + globals->pan.y;
        new_data["br"]["x"] = ((double)data["br"]["x"] * globals->zoom) + globals->pan.x;
        new_data["br"]["y"] = ((double)data["br"]["y"] * globals->zoom) + globals->pan.y;
    }
    if (data["type"] == "path")
    {
        for (int x = 0; x < data["points"].size(); x++)
        {
            new_data["points"][x]["x"] = (((double)data["points"][x]["x"] + globals->machine_parameters.work_offset[0]) * globals->zoom) + globals->pan.x;
            new_data["points"][x]["y"] = (((double)data["points"][x]["y"] + globals->machine_parameters.work_offset[1]) * globals->zoom) + globals->pan.y;
        }
    }
    return new_data;
}
std::string to_fixed_string(double n, int d)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(d) << n;
    return stream.str();
}
bool hmi_update_timer()
{
    nlohmann::json dro_data = motion_controller_get_dro();
    if (dro_data.contains("STATUS"))
    {
        // { "STATUS": "Idle", "MCS": { "x": 0.000,"y": 0.000,"z": 0.000 }, "WCS": { "x": -4.594,"y": -3.260,"z": 0.000 }, "FEED": 0, "ADC": 0, "IN_MOTION": false, "ARC_OK": true }
        dro.x.work_readout->data["textval"] = to_fixed_string(dro_data["WCS"]["x"], 4);
        dro.y.work_readout->data["textval"] = to_fixed_string(dro_data["WCS"]["y"], 4);
        dro.z.work_readout->data["textval"] = to_fixed_string(dro_data["WCS"]["z"], 4);
        dro.x.absolute_readout->data["textval"] = to_fixed_string(dro_data["MCS"]["x"], 4);
        dro.y.absolute_readout->data["textval"] = to_fixed_string(dro_data["MCS"]["y"], 4);
        dro.z.absolute_readout->data["textval"] = to_fixed_string(dro_data["MCS"]["z"], 4);
        dro.feed->data["textval"] = "FEED: " + to_fixed_string(dro_data["FEED"], 1);
        dro.arc_readout->data["textval"] = "ARC: " + to_fixed_string(dro_data["ADC"], 1);
        dro.arc_set->data["textval"] = "SET: " + to_fixed_string(0, 1);
        nlohmann::json runtime = motion_controller_get_run_time();
        if (runtime != NULL) dro.run_time->data["textval"] = "RUN: " + to_string(runtime["hours"]) + ":" + to_string(runtime["minutes"]) + ":" + to_string(runtime["seconds"]);

        globals->torch_pointer->data["center"] = {{"x", (double)dro_data["MCS"]["x"]}, {"y", (double)dro_data["MCS"]["y"]}};
    }
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
    group.button_one.object = Xrender_push_box({{"id", b1}, {"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 200},{"color", {{"r", 10},{"g", 10},{"b", 10},{"a", 255}}},});
    group.button_one.label = Xrender_push_text({{"textval", group.button_one.name},{"font", "default"},{"position", {{"x", -10000},{"y", -10000}}},{"font_size", 20},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    group.button_two.name = b2;
    group.button_two.object = Xrender_push_box({{"id", b2}, {"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 200},{"color", {{"r", 10},{"g", 10},{"b", 10},{"a", 255}}},});
    group.button_two.label = Xrender_push_text({{"textval", group.button_two.name},{"font", "default"},{"position", {{"x", -10000},{"y", -10000}}},{"font_size", 20},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    group.button_one.object->mouse_callback = mouse_callback;
    group.button_two.object->mouse_callback = mouse_callback;
    button_groups.push_back(group);
}
void tab_key_up(nlohmann::json e)
{
    if (way_point_position.x != -1000 & way_point_position.y != -1000)
    {
        LOG_F(INFO, "Going to waypoint position: X%.4f Y%.4f", way_point_position.x, way_point_position.y);
        motion_controller_push_stack("G53 G0 X" + to_string(way_point_position.x) + " Y" + to_string(way_point_position.y));
        motion_controller_push_stack("M30");
        motion_controller_run_stack();
        way_point_position.x = -1000;
        way_point_position.y = -1000;
        way_point_marker->data["center"] = {{"x", way_point_position.x}, {"y", way_point_position.y}};
        way_point_marker->data["visable"] = false;
    }
}
void hmi_init()
{
    globals->machine_plane = Xrender_push_box({{"tl", {{"x", 0},{"y", globals->machine_parameters.machine_extents[1]}}},{"br", {{"x", globals->machine_parameters.machine_extents[0]},{"y", 0}}},{"radius", 0},{"zindex", -20},{"color", {{"r", globals->preferences.machine_plane_color[0] * 255},{"g", globals->preferences.machine_plane_color[1] * 255},{"b", globals->preferences.machine_plane_color[2] * 255},{"a", 255}}},});
    globals->machine_plane->matrix_data = view_matrix;
    globals->cuttable_plane = Xrender_push_box({{"id", "cuttable_plane"}, {"tl", {{"x", globals->machine_parameters.cutting_extents[0]},{"y", globals->machine_parameters.machine_extents[1]+globals->machine_parameters.cutting_extents[3]}}},{"br", {{"x", globals->machine_parameters.machine_extents[0]+globals->machine_parameters.cutting_extents[2]},{"y", globals->machine_parameters.cutting_extents[1]}}},{"radius", 0},{"zindex", -10},{"color", {{"r", globals->preferences.cuttable_plane_color[0] * 255},{"g", globals->preferences.cuttable_plane_color[1] * 255},{"b", globals->preferences.cuttable_plane_color[2] * 255},{"a", 255}}},});
    globals->cuttable_plane->matrix_data = view_matrix;
    globals->cuttable_plane->mouse_callback = mouse_callback;
    
    hmi_backpane = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 0},{"zindex", 100},{"color", {{"r", 25},{"g", 44},{"b", 71},{"a", 255}}},});
    hmi_dro_backpane = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 100},{"color", {{"r", 29},{"g", 32},{"b", 48},{"a", 255}}},});
    hmi_button_backpane = Xrender_push_box({{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 5},{"zindex", 100},{"color", {{"r", 29},{"g", 32},{"b", 48},{"a", 255}}},});

    hmi_push_button_group("Zero X", "Zero Y");
    hmi_push_button_group("Touch", "Retract");
    hmi_push_button_group("Fit", "Clean");
    hmi_push_button_group("Wpos", "Park");
    hmi_push_button_group("Test Run", "ATHC");
    hmi_push_button_group("Run", "Abort");
 
    dro.x.label = Xrender_push_text({{"textval", "X"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    dro.x.work_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 40},{"zindex", 210},{"angle", 0},{"color", {{"r", 10},{"g", 150},{"b", 10},{"a", 255},}},});
    dro.x.absolute_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 15},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.x.divider = Xrender_push_box({{"id", "x_dro_divider"}, {"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 3},{"zindex", 150},{"color", {{"r", 0},{"g", 0},{"b", 0},{"a", 255}}},});

    dro.y.label = Xrender_push_text({{"textval", "Y"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    dro.y.work_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 40},{"zindex", 210},{"angle", 0},{"color", {{"r", 10},{"g", 150},{"b", 10},{"a", 255},}},});
    dro.y.absolute_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 15},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.y.divider = Xrender_push_box({{"id", "y_dro_divider"},{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 3},{"zindex", 150},{"color", {{"r", 0},{"g", 0},{"b", 0},{"a", 255}}},});

    dro.z.label = Xrender_push_text({{"textval", "Z"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 50},{"zindex", 210},{"angle", 0},{"color", {{"r", 255},{"g", 255},{"b", 255},{"a", 255},}},});
    dro.z.work_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 40},{"zindex", 210},{"angle", 0},{"color", {{"r", 10},{"g", 150},{"b", 10},{"a", 255},}},});
    dro.z.absolute_readout = Xrender_push_text({{"textval", "0.0000"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 15},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.z.divider = Xrender_push_box({{"id", "z_dro_divider"},{"tl", {{"x", -100000},{"y", -100000}}},{"br", {{"x", -100000},{"y", -100000}}},{"radius", 3},{"zindex", 150},{"color", {{"r", 0},{"g", 0},{"b", 0},{"a", 255}}},});

    dro.feed = Xrender_push_text({{"textval", "FEED: 0.0"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.arc_readout = Xrender_push_text({{"textval", "ARC: 0.0"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.arc_set = Xrender_push_text({{"textval", "SET: 0.0"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});
    dro.run_time = Xrender_push_text({{"textval", "RUN: 00:00:00"}, {"font", "default"}, {"position", {{"x", -10000}, {"y", -10000}}},{"font_size", 12},{"zindex", 210},{"angle", 0},{"color", {{"r", 247},{"g", 104},{"b", 15},{"a", 255},}},});

    globals->torch_pointer = Xrender_push_circle({{"center", {{"x", -1000},{"y", -1000}}},{"color", {{"r", 0},{"g", 255},{"b", 0},{"a", 255},}},{"radius", 5},{"zindex", 500},{"id", "torch_pointer"},});
    globals->torch_pointer->matrix_data = &view_matrix;

    way_point_marker = Xrender_push_circle({{"center", {{"x", -1000},{"y", -1000}}},{"color", {{"r", 0},{"g", 0},{"b", 255},{"a", 255},}},{"radius", 5},{"zindex", 500},{"id", "way_point_marker"},{"visable", false}});
    way_point_marker->matrix_data = &view_matrix;

   
    //Xrender_push_key_event({"Escape", "keyup", escape_key_up});
    Xrender_push_key_event({"Tab", "keyup", tab_key_up});
    Xrender_push_key_event({"none", "window_resize", hmi_resize});
    Xrender_push_timer(100, hmi_update_timer);
    hmi_handle_button("Fit");
}