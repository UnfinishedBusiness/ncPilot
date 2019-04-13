#include "Arduino.h"
#include "Motion.h"
#include "StepGen.h"

int axis_counter;
axis_t axis[MAX_NUMBER_OF_AXIS];

void motion_init()
{
  axis_counter = 0;
}

void motion_init_axis(int step_gen, char axis_letter, float max_accel, float scale)
{
  axis[axis_counter].stepgen_number = step_gen;
  axis[axis_counter].axis_letter = axis_letter;
  axis[axis_counter].max_accel = max_accel;
  axis[axis_counter].scale = scale;
  axis[axis_counter].absolute_position = 0;
  axis[axis_counter].distance_to_go = 0;
  axis[axis_counter].current_velocity = 0;
  axis_counter++;
}
void motion_tick()
{
  for (int x = 0; x < axis_counter; x++)
  {
    axis[x].current_velocity = stepgen_get_velocity(axis[x].stepgen_number) / axis[x].scale;
    axis[x].absolute_position = stepgen_get_position(axis[x].stepgen_number) / axis[x].scale;
  }
}
void motion_plan_target(char *target_words, float feedrate)
{
  char axis_word;
  char number[10];
  int num_p;
  float axis_value;
  int x;
  while(true)
  {
      if (target_words[x] == '\0') break;
      if (isalpha(target_words[x]) && target_words[x] != '-' && target_words[x] != '.') //We are a Axis leter
      {
        axis_word = target_words[x];
        number[0] = '\0';
        num_p = 0;
        do{
              x++;
              number[num_p] = target_words[x];
              num_p++;
        }while(isdigit(target_words[x]) || target_words[x] == '.' || target_words[x] == '-');
        number[num_p-1] = '\0';
        x--;
        axis_value = atof(number);
        for (int x = 0; x < axis_counter; x++)
        {
          if (toupper(axis[x].axis_letter) == toupper(axis_word))
          {
            axis[x].target_position = axis_value;
          }
        }
      }
      x++;
  }

}
