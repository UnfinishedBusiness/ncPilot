#include <Xrender.h>
#include <application.h>
#include "event_handling/event_handling.h"
#include "motion_control/motion_control.h"
#include "hmi/hmi.h"
#include "logging/loguru.h"
#include <json/json.h>
#include <stdio.h>
#include <iostream>


void event_handling_escape_key(nlohmann::json e)
{
    hmi_handle_button("Abort");
    globals->way_point_marker->data["visable"] = false;
    globals->way_point_position = {-1000, -1000};
}
void event_handling_mouse_motion(nlohmann::json e)
{
    globals->mouse_pos_in_screen_coordinates = {(double)e["pos"]["x"], (double)e["pos"]["y"]};
    globals->mouse_pos_in_matrix_coordinates = {
        ((double)e["pos"]["x"] / globals->zoom) - (globals->pan.x / globals->zoom),
        ((double)e["pos"]["y"] / globals->zoom) - (globals->pan.y / globals->zoom)
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
    //LOG_F(INFO, "Zoom-> %.4f", globals->zoom);
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
    //LOG_F(INFO, "Zoom-> %.4f", globals->zoom);
}
void event_handling_up_key(nlohmann::json e)
{
    if ((int)e["action"] == 1)
    {
        //key down
        LOG_F(INFO, "Jogging Y positive!");
        motion_controller_push_stack("G53 G0 Y" + to_string(globals->machine_parameters.machine_extents[1]));
        motion_controller_run_stack();
    }
    if ((int)e["action"] == 0)
    {
        //key up
        LOG_F(INFO, "Cancelling Y positive jog!");
        hmi_handle_button("Abort");
    }
}
void event_handling_down_key(nlohmann::json e)
{
    if ((int)e["action"] == 1)
    {
        //key down
        LOG_F(INFO, "Jogging Y negative!");
        motion_controller_push_stack("G53 G0 Y0");
        motion_controller_run_stack();
    }
    if ((int)e["action"] == 0)
    {
        //key up
        LOG_F(INFO, "Cancelling Y negative jog!");
        hmi_handle_button("Abort");
    }
}
void event_handling_right_key(nlohmann::json e)
{
    if ((int)e["action"] == 1)
    {
        //key down
        LOG_F(INFO, "Jogging X positive!");
        motion_controller_push_stack("G53 G0 X" + to_string(globals->machine_parameters.machine_extents[0]));
        motion_controller_run_stack();
    }
    if ((int)e["action"] == 0)
    {
        //key up
        LOG_F(INFO, "Cancelling X positive jog!");
        hmi_handle_button("Abort");
    }
}
void event_handling_left_key(nlohmann::json e)
{
    if ((int)e["action"] == 1)
    {
        //key down
        LOG_F(INFO, "Jogging X negative!");
        motion_controller_push_stack("G53 G0 X0");
        motion_controller_run_stack();
    }
    if ((int)e["action"] == 0)
    {
        //key up
        LOG_F(INFO, "Cancelling X negative jog!");
        hmi_handle_button("Abort");
    }
}
void event_handling_init()
{
    Xrender_push_key_event({"up", "scroll", event_handling_zoom_in});
    Xrender_push_key_event({"down", "scroll", event_handling_zoom_out});
    Xrender_push_key_event({"none", "mouse_move", event_handling_mouse_motion});
    Xrender_push_key_event({"Escape", "keyup", event_handling_escape_key});

    Xrender_push_key_event({"Up", "keydown", event_handling_up_key});
    Xrender_push_key_event({"Up", "keyup", event_handling_up_key});
    Xrender_push_key_event({"Down", "keydown", event_handling_down_key});
    Xrender_push_key_event({"Down", "keyup", event_handling_down_key});
    Xrender_push_key_event({"Left", "keydown", event_handling_left_key});
    Xrender_push_key_event({"Left", "keyup", event_handling_left_key});
    Xrender_push_key_event({"Right", "keydown", event_handling_right_key});
    Xrender_push_key_event({"Right", "keyup", event_handling_right_key});
}