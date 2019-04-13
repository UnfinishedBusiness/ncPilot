#include "Arduino.h"
#include "StepGen.h"
#include "CircularBuffer.h"

IntervalTimer StepgenTimer;

int number_of_stepgens;
stepgen_t StepGen[MAX_NUMBER_OF_STEPGENS];

int step_counters[MAX_NUMBER_OF_STEPGENS];


CircularBuffer<stepgen_segment_t, MAX_NUMBER_OF_SEGMENTS> segment_buffer;

/* Facilitate High Priority Step Train timing */
void stepgen_timer_tick()
{
  if (segment_buffer.size() > 0)
  {
    int move_finished = 0;
    //Iterate over each stepgen to see if there are steps to eat
    for (int x = 0; x < number_of_stepgens; x++)
    {
      if (((micros() - StepGen[x].cycle_timestamp) > segment_buffer.first().segment_rate[x]) && (step_counters[x] < abs(segment_buffer.first().steps_to_move[x])))
      {
          StepGen[x].cycle_timestamp = micros();
          if (segment_buffer.first().steps_to_move[x] > 0) //We are a positive move
          {
            //digitalWrite(StepGen[x].dir_pin, HIGH);
            step_counters[x]++;
            StepGen[x].position++;
          }
          else
          {
            //digitalWrite(StepGen[x].dir_pin, LOW);
            step_counters[x]++;
            StepGen[x].position--;
          }
          digitalWrite(StepGen[x].step_pin, HIGH);
      }
      else if (step_counters[x] == abs(segment_buffer.first().steps_to_move[x]))
      {
        move_finished++;
      }
      else
      {
        digitalWrite(StepGen[x].step_pin, LOW);
      }
    }
    if (move_finished == number_of_stepgens) //Eat top move of stack
    {
      //Serial.println("Segment finished!");
      segment_buffer.shift(); //Eat first segment. First is now next segment
      for (int x = 0; x < number_of_stepgens; x++)
      {
        step_counters[x] = 0;
        StepGen[x].velocity = segment_buffer.first().segment_velocity[x];
      }
    }
  }
}
accel_t stepgen_plan_acceleration(float initial_velocity, float target_velocity, float acceleration_rate, float scale)
{
  accel_t ret;
  float velocity = initial_velocity;
  ret.accel_time = (target_velocity - initial_velocity) / acceleration_rate;
  ret.accel_distance = 0.5 * ((target_velocity + initial_velocity) * ret.accel_time);
  ret.velocity_inc_per_cycle = (float)(target_velocity - initial_velocity) / (float)(ret.accel_distance / SEGMENT_LENGTH);
  ret.number_of_cycles = (int)(ret.accel_distance / SEGMENT_LENGTH);
  for (int x = 0; x < ret.number_of_cycles; x++)
  {
    ret.steps_to_move = scale * SEGMENT_LENGTH;
    ret.segment_rate = (velocity * scale);
    ret.segment_velocity = (int)(1000000.0 / (velocity * scale));
    velocity += ret.velocity_inc_per_cycle;
  }
  return ret;
}
accel_t stepgen_plan_decceleration(float initial_velocity, float target_velocity, float acceleration_rate, float scale)
{
  accel_t ret;
  float velocity = initial_velocity;
  ret.accel_time = (initial_velocity - target_velocity) / acceleration_rate;
  ret.accel_distance = 0.5 * ((target_velocity + initial_velocity) * ret.accel_time);
  ret.velocity_inc_per_cycle = (float)(initial_velocity - target_velocity) / (float)(ret.accel_distance / SEGMENT_LENGTH);
  ret.number_of_cycles = (int)(ret.accel_distance / SEGMENT_LENGTH);
  for (int x = 0; x < ret.number_of_cycles; x++)
  {
    ret.steps_to_move = scale * SEGMENT_LENGTH;
    ret.segment_rate = (velocity * scale);
    ret.segment_velocity = (int)(1000000.0 / (velocity * scale));
    velocity += ret.velocity_inc_per_cycle;
  }
  return ret;
}
void stepgen_init(int stepgens)
{
  if (stepgens <= MAX_NUMBER_OF_STEPGENS)
  {
    number_of_stepgens = stepgens;
    for (int x = 0; x < number_of_stepgens; x++)
    {
      StepGen[x].position = 0;
      StepGen[x].velocity = 0;
    }
    StepgenTimer.begin(stepgen_timer_tick, 10);
  }
}
void stepgen_init_gen(int stepgen, int step_pin, int dir_pin)
{
  if (stepgen < number_of_stepgens)
  {
    StepGen[stepgen].step_pin = step_pin;
    StepGen[stepgen].dir_pin = dir_pin;
  }
}
int stepgen_get_position(int gen)
{
  return StepGen[gen].position;
}
int stepgen_get_velocity(int gen)
{
  return StepGen[gen].velocity;
}
bool stepgen_push_segment_to_stack(stepgen_segment_t segment)
{
  while(segment_buffer.available() < 100)
  {
    //Serial.print("Circular Buffer is full, wiating until there is room! ");
    //Serial.println(micros());
    delay(1); //This delay need to be here or we lock up for some reason??
  } //Don't add any data until there is room in the buffer
  segment_buffer.push(segment);
}
