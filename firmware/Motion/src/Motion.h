#ifndef MOTION_H
#define MOTION_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
 #define MAX_NUMBER_OF_AXIS 6

 #define LINEAR_AXIS 0
 #define ANGULAR_AXIS 1

 #define ACCEL_TICK_PERIOD 1000

/**********************
 *      TYPEDEFS
 **********************/
 typedef struct
 {
   /* Motion Parameters */
   float min_feed_rate;
   float max_linear_velocity;
   int number_of_axis;
   /* ----------------- */

   float entry_velocity;
   float target_velocity;
   float current_velocity;
   float exit_velocity;
   float total_move_distance;
   float distance_into_move;
   float distance_to_go;

   bool InMotion;
   /* ---------------- */

   float acceleration_marker;
   float decceleration_marker;
   float acceleration_rate_per_cycle;

   float accel_time;
   float deccel_time;

   unsigned long move_start_timestamp;
 }motion_t;

 typedef struct
 {
   /* Axis Parameters */
   int axis_type;
   int step_pin;
   int dir_pin;
   bool move_direction;
   char axis_letter;
   char *scale_units;
   float scale;
   float inverse_scale;
   float max_accel;
   float max_velocity;

   /* Positional Data */
   float current_position;

   /* Target Data */
   float target_position;

   /* Used for interpolation */
   float current_velocity;
   float initial_velocity;

   int total_move_steps;
   int steps_left_to_travel;
   unsigned long cycle_timestamp;
   unsigned long cycle_speed;
 }axis_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
 void motion_init_axis(int, int, int, int, char, char*, float, float, float);
 void motion_init(int, float, float);
 void motion_set_target_position(char*, float, float);
 void motion_timer_tick();
 void motion_accel_tick();
 void motion_loop_tick();
/**********************
 * CONTROLS PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*AXIS_H*/
