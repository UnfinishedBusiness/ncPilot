#ifndef APPLICATION_
#define APPLICATION_

#include <Xrender.h>
/*
    Primary structure to store global variables in
*/
struct preferences_data_t{
    float background_color[3] = { 0.0f, 0.0f, 0.0f };
    float machine_plane_color[3] = { 0.0f, 0.0f, 0.0f };
    float cuttable_plane_color[3] = { 0.0f, 0.0f, 0.0f };
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
};
extern global_variables_t *globals;

#endif //APPLICATION_