#ifndef APPLICATION_
#define APPLICATION_

#include <Xrender.h>
#include <json/json.h>
/*
    Primary structure to store global variables in
*/
struct preferences_data_t{
    float background_color[3] = { 0.0f, 0.0f, 0.0f };
    float machine_plane_color[3] = { 0.0f, 0.0f, 0.0f };
    float cuttable_plane_color[3] = { 0.0f, 0.0f, 0.0f };
};
struct machine_parameters_data_t{
    float machine_extents[3] = { 0.0f, 0.0f, 0.0f };
    float cutting_extents[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    float axis_scale[3] = { 0.0f, 0.0f, 0.0f };
    float max_vel[3] = { 0.0f, 0.0f, 0.0f };
    float max_accel[3] = { 0.0f, 0.0f, 0.0f };
    float junction_deviation = 0;
    float floating_head_backlash = 0;
    float z_probe_feedrate = 0;
    bool axis_invert[4] = { false, false, false, false };
};
struct global_variables_t{
    bool quit;
    double zoom;
    double_point_t pan;
    double_point_t mouse_pos_in_screen_coordinates;
    double_point_t mouse_pos_in_matrix_coordinates;
    Xrender_core_t *Xcore;
    preferences_data_t preferences;
    Xrender_object_t *machine_plane;
    Xrender_object_t *cuttable_plane;
    machine_parameters_data_t machine_parameters;
};

nlohmann::json view_matrix(nlohmann::json data);
extern global_variables_t *globals;

#endif //APPLICATION_