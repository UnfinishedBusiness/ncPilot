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

   /* Variables updated by sample check and used by move */
   float target_velocity;
   float linear_velocity;
   unsigned long move_start_timestamp;
   unsigned long move_decel_timestamp;
   bool InMotion;
   float decceleration_dtg_marker;
   unsigned long time_required_to_accelerate;
   /* ---------------- */

 }motion_t;

 typedef struct
 {
   /* Axis Parameters */
   int axis_type;
   int step_pin;
   int dir_pin;
   char axis_letter;
   char *scale_units;
   float scale;
   float max_accel;
   float max_velocity;

   /* Positional Data */
   float current_position;

   /* Target Data */
   float target_position;

   /* Used for interpolation */
   float current_velocity;
   float initial_velocity;
   int steps_left_to_travel;
   unsigned long cycle_timestamp;
   unsigned long cycle_speed_at_min_feed_rate;
   unsigned long cycle_speed;
 }axis_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
 void motion_init_axis(int, int, int, int, char, char*, float, float, float);
 void motion_init(int, float, float);
 void motion_set_target_position(char*, float);
 void motion_timer_tick();
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
