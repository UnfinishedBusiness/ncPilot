#ifndef STEPGEN_H
#define STEPGEN_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
#define MAX_NUMBER_OF_STEPGENS 2
#define MAX_NUMBER_OF_SEGMENTS 2000
/**********************
 *      TYPEDEFS
 **********************/
 typedef struct
 {
   int step_pin;
   int dir_pin;

   //These variables are updated by the timer
   long position; //Current position in steps
   float velocity; //In steps per second
   unsigned long cycle_timestamp;
 }stepgen_t;

 typedef struct
 {
   int steps_to_move[MAX_NUMBER_OF_STEPGENS]; //Move this number of steps, dir indicated by negative/positive sign
   int segment_speed[MAX_NUMBER_OF_STEPGENS]; //This needs to be the length of time between steps
   int segment_velocity[MAX_NUMBER_OF_STEPGENS]; //This needs to be in steps / second
 }stepgen_segment_t;

 typedef struct
 {
   float accel_time;
   float accel_distance;
   float velocity_inc_per_cycle;
   int number_of_cycles;

   int steps_to_move;
   int segment_rate;
   int segment_velocity;
 }accel_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
 void stepgen_init(int);
 void stepgen_init_gen(int, int, int);
 bool stepgen_push_segment_to_stack(stepgen_segment_t);
 long stepgen_get_position(int);
 long stepgen_get_velocity(int);
 accel_t stepgen_plan_acceleration(float, float, float, float);
 accel_t stepgen_plan_decceleration(float, float, float, float);
/**********************
 * CONTROLS PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*STEPGEN_H*/
