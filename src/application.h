#ifndef APPLICATION_
#define APPLICATION_

#include <Xrender.h>
/*
    Primary structure to store global variables in
*/
struct global_variables_t{
    bool quit;
    double zoom;
    double_point_t pan;
    double_point_t mouse_pos_in_screen_coordinates;
    double_point_t mouse_pos_in_matrix_coordinates;
};
extern global_variables_t *globals;

#endif //APPLICATION_