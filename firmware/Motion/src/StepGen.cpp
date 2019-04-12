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
      if (((micros() - StepGen[x].cycle_timestamp) > segment_buffer.first().segment_speed[x]) && (step_counters[x] < abs(segment_buffer.first().steps_to_move[x])))
      {
          StepGen[x].cycle_timestamp = micros();
          if (segment_buffer.first().steps_to_move[x] > 0) //We are a positive move
          {
            digitalWrite(StepGen[x].dir_pin, HIGH);
            step_counters[x]++;
            StepGen[x].position++;
          }
          else
          {
            digitalWrite(StepGen[x].dir_pin, LOW);
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
      for (int x = 0; x < number_of_stepgens; x++) step_counters[x] = 0;
    }
  }
}

void stepgen_init(int stepgens)
{
  if (stepgens <= MAX_NUMBER_OF_STEPGENS)
  {
    number_of_stepgens = stepgens;
    for (int x = 0; x < number_of_stepgens; x++)
    {
      StepGen[x].position = 0;
      StepGen[x].step_speed = 0;
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
bool stepgen_push_segment_to_stack(stepgen_segment_t segment)
{
    return segment_buffer.push(segment);
}
