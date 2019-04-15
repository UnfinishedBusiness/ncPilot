#include "Arduino.h"
#include "Motion.h"
#include "StepGen.h"
#include "Machine.h"

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
void motion_parse_current_position_words(char *target_words)
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
            axis[x].planned_current_position = axis_value;
          }
        }
      }
      x++;
  }
}
void motion_parse_target_position_words(char *target_words)
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
            axis[x].planned_target_position = axis_value;
          }
        }
      }
      x++;
  }
}
void _debug_print_parsed_axis_positions()
{
  //Current Positions
  Serial.println("Planned Current Position");
  for (int x = 0; x < axis_counter; x++)
  {
    Serial.print(axis[x].axis_letter);
    Serial.print(" = ");
    Serial.println(axis[x].planned_current_position, 4);
  }
  //Target Positions
  Serial.println("Planned Target Position");
  for (int x = 0; x < axis_counter; x++)
  {
    Serial.print(axis[x].axis_letter);
    Serial.print(" = ");
    Serial.println(axis[x].planned_target_position, 4);
  }
}
float motion_get_planned_current_position_by_letter(char letter)
{
  for (int x = 0; x < axis_counter; x++)
  {
    if (axis[x].axis_letter == letter) return axis[x].planned_current_position;
  }
}
float motion_get_planned_target_position_by_letter(char letter)
{
  for (int x = 0; x < axis_counter; x++)
  {
    if (axis[x].axis_letter == letter) return axis[x].planned_target_position;
  }
}
float motion_get_position_diff_by_letter(char letter)
{
  for (int x = 0; x < axis_counter; x++)
  {
    if (axis[x].axis_letter == letter) return fabs(axis[x].planned_current_position - axis[x].planned_target_position);
  }
}
int motion_get_axis_with_biggest_planned_move()
{
  int ret = 0;
  float diff = 0;
  for (int x = 0; x < axis_counter; x++)
  {
    if (fabs(axis[x].planned_current_position - axis[x].planned_target_position) > diff)
    {
      diff = fabs(axis[x].planned_current_position - axis[x].planned_target_position);
      ret = x;
    }
  }
  return ret;
}
void motion_plan_move(char *current_position, char* target_position, float entry_velocity, float target_velocity, float exit_velocity)
{
  stepgen_segment_t segment;
  motion_parse_current_position_words(current_position);
  motion_parse_target_position_words(target_position);
  _debug_print_parsed_axis_positions();
  int biggest_axis = motion_get_axis_with_biggest_planned_move();
  Serial.print("Biggest Axes: ");
  Serial.println(axis[biggest_axis].axis_letter);
  float target_accel = 30.0;
  accel_t accel[axis_counter];
  float constraint_scale[axis_counter];
  for (int x = 0; x < axis_counter; x ++)
  {
    constraint_scale[x] = 0;
  }
  for (int x = 0; x < axis_counter; x ++)
  {
    constraint_scale[x] = motion_get_position_diff_by_letter(axis[x].axis_letter) / motion_get_position_diff_by_letter(axis[biggest_axis].axis_letter);
    accel[x] = stepgen_plan_acceleration(entry_velocity, target_velocity * constraint_scale[x], target_accel * constraint_scale[x]);
    Serial.print(axis[x].axis_letter);
    Serial.print("-> Target Velocity: ");
    Serial.println(accel[x].target_velocity, 4);
    Serial.print(axis[x].axis_letter);
    Serial.print("-> Accel Distance: ");
    Serial.println(accel[x].accel_distance, 4);
  }
  //Find the axis that has to travel the farthest. Then divide each axis target position by the big axis. This is the scale factor for each axis' target feedrate
  // and acceleration rate
  int number_of_cycles = (int)(accel[biggest_axis].accel_distance / SEGMENT_LENGTH);
  Serial.print("number_of_cycles: ");
  Serial.println(number_of_cycles);
  float velocity_inc_per_cycle[axis_counter];
  for (int x = 0; x < axis_counter; x ++)
  {
    velocity_inc_per_cycle[x] = (float)(accel[x].target_velocity - accel[x].initial_velocity) / number_of_cycles;
    Serial.print(axis[x].axis_letter);
    Serial.print(" Velocity inc per cycle -> ");
    Serial.println(velocity_inc_per_cycle[x]);
  }
  float segment_velocity[axis_counter];
  for (int x = 0; x < axis_counter; x ++)
  {
    segment_velocity[x] = entry_velocity;
  }
  for (int x = 0; x < number_of_cycles; x++)
  {
    for (int y = 0; y < axis_counter; y++)
    {
      segment.steps_to_move[axis[y].stepgen_number] = ((axis[y].scale * SEGMENT_LENGTH) * constraint_scale[y]);
      segment.segment_velocity[axis[y].stepgen_number] = (segment_velocity[y] * axis[y].scale);
      segment.segment_rate[axis[y].stepgen_number] = (int)(1000000.0 / (segment_velocity[y] * axis[y].scale));
      segment_velocity[y] += velocity_inc_per_cycle[y];
    }
    stepgen_push_segment_to_stack(segment);
  }

}
