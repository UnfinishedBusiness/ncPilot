#include <Xrender.h>
#include <application.h>
#include "event_handling/event_handling.h"
#include <stdio.h>
#include <iostream>

void event_handling_mouse_motion(nlohmann::json e)
{
    //printf("%s\n", e.dump().c_str());
    globals->mouse_pos_in_screen_coordinates = {(double)e["pos"]["x"], (double)e["pos"]["y"]};
    globals->mouse_pos_in_matrix_coordinates = {
        (globals->mouse_pos_in_screen_coordinates.x - globals->pan.x) / globals->zoom,
        (globals->mouse_pos_in_screen_coordinates.y - globals->pan.y) / globals->zoom
    };
}
void event_handling_zoom_in(nlohmann::json e)
{
    double old_zoom = globals->zoom;
    globals->zoom += globals->zoom * 0.125;
    if (globals->zoom > 1000000)
    {
        globals->zoom = 1000000;
    }
    double scalechange = old_zoom - globals->zoom;
    globals->pan.x += globals->mouse_pos_in_matrix_coordinates.x * scalechange;
    globals->pan.y += globals->mouse_pos_in_matrix_coordinates.y * scalechange; 
}
void event_handling_zoom_out(nlohmann::json e)
{
    double old_zoom = globals->zoom;
    globals->zoom += globals->zoom * -0.125;
    if (globals->zoom < 0.00001)
    {
        globals->zoom = 0.00001;
    }
    double scalechange = old_zoom - globals->zoom;
    globals->pan.x += globals->mouse_pos_in_matrix_coordinates.x * scalechange;
    globals->pan.y += globals->mouse_pos_in_matrix_coordinates.y * scalechange; 
}
void event_handling_init()
{
    Xrender_push_key_event({"up", "scroll", event_handling_zoom_in});
    Xrender_push_key_event({"down", "scroll", event_handling_zoom_out});
    Xrender_push_key_event({"none", "mouse_move", event_handling_mouse_motion});
}