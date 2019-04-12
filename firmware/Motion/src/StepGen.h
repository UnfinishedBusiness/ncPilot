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
   float step_speed; //In steps per second
   unsigned long cycle_timestamp;
 }stepgen_t;

 typedef struct
 {
   int steps_to_move[MAX_NUMBER_OF_STEPGENS]; //Move this number of steps, dir indicated by negative/positive sign
   int segment_speed[MAX_NUMBER_OF_STEPGENS];
 }stepgen_segment_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
 void stepgen_init(int);
 void stepgen_init_gen(int, int, int);
 bool stepgen_push_segment_to_stack(stepgen_segment_t);
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
