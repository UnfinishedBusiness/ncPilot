#ifndef APPLICATION_
#define APPLICATION_

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/types.h>
#include <dirent.h>
#include <ftw.h>
#include "EasyRender/EasyRender.h"


/*
    Primary structure to store global variables in
*/
struct preferences_data_t{
    float background_color[3] = { 0.0f, 0.0f, 0.0f };
    float machine_plane_color[3] = { 0.0f, 0.0f, 0.0f };
    float cuttable_plane_color[3] = { 0.0f, 0.0f, 0.0f };
    int window_size[2] = { 0, 0 };
};
struct machine_parameters_data_t{
    float work_offset[3] = { 0.0f, 0.0f, 0.0f };
    float machine_extents[3] = { 0.0f, 0.0f, 0.0f };
    float cutting_extents[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float axis_scale[3] = { 0.0f, 0.0f, 0.0f };
    float max_vel[3] = { 0.0f, 0.0f, 0.0f };
    float max_accel[3] = { 0.0f, 0.0f, 0.0f };
    float junction_deviation = 0;
    float floating_head_backlash = 0;
    float z_probe_feedrate = 0;
    float arc_stablization_time = 0;
    bool axis_invert[4] = { false, false, false, false };
    //Not Included in machine_parameters.json
    int thc_set_value = 0;
    bool smart_thc_on = false;
};
struct global_variables_t{
    bool quit;
    double zoom;
    double_point_t pan;
    double_point_t mouse_pos_in_screen_coordinates;
    double_point_t mouse_pos_in_matrix_coordinates;
    EasyRender *renderer;
    preferences_data_t preferences;
    EasyPrimative::Box *machine_plane;
    EasyPrimative::Box *cuttable_plane;
    EasyPrimative::Circle *torch_pointer;
    machine_parameters_data_t machine_parameters;
    double_point_t way_point_position;
    unsigned long start_timestamp;
};
extern global_variables_t *globals;

#endif //APPLICATION_