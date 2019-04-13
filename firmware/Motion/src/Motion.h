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
/**********************
 *      TYPEDEFS
 **********************/
 typedef struct
 {
   /* Axis Parameters */
   int stepgen_number;
   char axis_letter;
   float max_accel;
   float scale;

   /* Target Data */
   float target_position;

   /* Real-Time Information */
   float absolute_position; //At scale
   float distance_to_go;
   float current_velocity; //In scale/sec

 }axis_t;
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void motion_init();
void motion_init_axis(int, char, float, float);
void motion_tick();
void motion_plan_target(char *, float);
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
