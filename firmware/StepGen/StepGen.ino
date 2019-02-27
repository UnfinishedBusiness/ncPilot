#include <stdio.h>

#define X_STEP 2
#define Y_STEP 3
#define Z_STEP 4
#define X_DIR 5
#define Y_DIR 6
#define Z_DIR 7

#define ONE_MINUTE 60000000 //60,000,000
#define STEP_LEN 10 //10 uS
#define MOTION_BUFFER_SIZE 20
#define VELOCITY_UPDATE_PERIOD 10 //update velocity every 'x'ms while accelerating

int axis_scale[3]; //Steps per inch
int axis_max_acceleration[3]; //Acceleration in inch/sec^2
float interpolation_accel;

void write_pin(byte pin, bool value) //Costs about 4 uS
{
  if (value == LOW)
  {
    PORTD &= ~(1<<pin);
  }
  else
  {
    PORTD |= 1<<pin;
  }
}


unsigned long x_step_timer = 0;
unsigned long y_step_timer = 0;
unsigned long z_step_timer = 0;
unsigned long velocity_change_timer = 0;
char c;
char buff[100];
int buff_pos;

/*
 * Serial Protocal
 * to add a motion plan to the buffer ">XSTEPS:DELAY|YSTEPS:DELAY|ZSTEPS:DELAY\n"
 * to set io "!PIN|VALUE\n"
 * to read io "?PIN\n" returns one byte imediantly of pin value
 * to hold "-\n"
 * to start from previous hold "+\n"
 * to clear motion buffer "#\n"
 */


typedef struct
{
    float x_dist;
    float y_dist;
    float z_dist;
    float target_vel;
    bool vel_dir; //True is + and false is -
    bool reached_target_vel;
    long x_step_count;
    long y_step_count;
    long z_step_count;

    bool x_dir;
    bool y_dir;
    bool z_dir;

    unsigned long move_began_at; //This is a timestamp of when the move started. This is used to calculate the acceleration during the move
    unsigned long reached_max_accel_at;
} move_t;

unsigned long x_timer;
unsigned long y_timer;
unsigned long z_timer;

unsigned long x_delay;
unsigned long y_delay;
unsigned long z_delay;
float current_velocity;

move_t move_buffer[MOTION_BUFFER_SIZE];

float CalculateVelocity(float initial_vel, float time_in_move)
{
  //Serial.print("Time into move: ");
  //Serial.println(time_in_move);
  //Time is in milliseconds. So while traveling at 'x' distance over a period of 'time' ms our velocity is returned in inches/min
  //vel = init_vel + (acceleration * time)
  return initial_vel + (interpolation_accel) * (time_in_move / 6000);
}
struct feedrate_t
{
    unsigned long x;
    unsigned long y;
    unsigned long z;
    bool bad_calculation;
};
struct feedrate_t getIndividualAxisFeedrates(float f, float x_dist, float y_dist, float z_dist)
{
  if (f < 1) f = 2;
  feedrate_t r;

  unsigned long x_feed = 0;
  unsigned long y_feed = 0;
  unsigned long z_feed = 0;
  float cartesion_dist = (float) sqrt(pow(fabs(x_dist), 2) + pow(fabs(y_dist), 2) + pow(fabs(z_dist), 2));
  //System.out.println("Cartesion Distance: " + cartesion_dist);
  unsigned long move_will_take = (unsigned long) ((cartesion_dist / f) * ONE_MINUTE); //Move will take 'x' nanoseconds to finish at specified feedrate
  x_feed = (unsigned long)(move_will_take / ((float)axis_scale[0] * fabs(x_dist)));
  y_feed = (unsigned long)(move_will_take / ((float)axis_scale[1] * fabs(y_dist)));
  z_feed = (unsigned long)(move_will_take / ((float)axis_scale[2] * fabs(z_dist)));

  //if (x_feed > 9000000000000000000l) x_feed = 0;
  //if (y_feed > 9000000000000000000l) y_feed = 0;
  //if (z_feed > 9000000000000000000l) z_feed = 0;

  if (x_feed < 0) x_feed = 1000000;
  if (y_feed < 0) y_feed = 1000000;
  if (z_feed < 0) z_feed = 1000000;

  r.bad_calculation = false;
  if (x_feed > ONE_MINUTE) r.bad_calculation = true;
  if (y_feed > ONE_MINUTE) r.bad_calculation = true;
  if (z_feed > ONE_MINUTE) r.bad_calculation = true;

  //Serial.print("x_feed: ");
  //Serial.println(x_feed);

  //Serial.print("y_feed: ");
  //Serial.println(y_feed);

  //Serial.print("z_feed: ");
  //Serial.println(z_feed);


  r.x = x_feed;
  r.y = y_feed;
  r.z = z_feed;
  return r;
}

int move_bottom_pos;
void next_move()
{
  /* Shift all moves up one possition on the stack, current move is always index 0 */
  if (move_bottom_pos > 0)
  {
    for (int x = 1; x < MOTION_BUFFER_SIZE; x++)
    {
      move_buffer[x-1] = move_buffer[x];
    }
    move_bottom_pos--;
    move_buffer[0].move_began_at = millis();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(X_STEP, OUTPUT);
  pinMode(Y_STEP, OUTPUT);
  pinMode(Z_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(Y_DIR, OUTPUT);
  pinMode(Z_DIR, OUTPUT);

  buff_pos = 0;
  move_bottom_pos = 0;

  axis_scale[0] = 1250;
  axis_scale[1] = 1250;
  axis_scale[2] = 300;


  axis_max_acceleration[0] = 10;
  axis_max_acceleration[1] = 10;
  axis_max_acceleration[2] = 10;

  interpolation_accel = 30;

  x_timer = micros();
  y_timer = micros();
  z_timer = micros();
}
void parseBuffer()
{
  //Serial.println("Parsing buffer!");
  move_t m;
  char num_buf[10];
  int num_pos = 0;
  for (int x = 0; x < buff_pos; x++)
  {
    if (buff[x] == '>') //We are a motion profile that needs to go on the stack!
    {
      //>XDIST|YDIST|ZDIST|INIT_VEL|BASE_VEL|EXIT_VEL
      //Serial.println("Found motion command!");
      x++; //Move past ">"

      /* Parse XDIST */
      num_pos = 0;
      while(x < buff_pos)
      {
        num_buf[num_pos] = buff[x];
        num_pos++;
        x++;
        if (buff[x] == ':' || buff[x] == '|')
        {
          x++; //move past delimmeter
          break;
        }
      }
      m.x_dist = atof(num_buf);
      //Serial.print("XDIST: ");
      //Serial.println(m.x_dist);
      for (int x = 0; x < num_pos; x++) num_buf[x] = '\0';

      /* Parse YDIST */
      num_pos = 0;
      while(x < buff_pos)
      {
        num_buf[num_pos] = buff[x];
        num_pos++;
        x++;
        if (buff[x] == ':' || buff[x] == '|')
        {
          x++; //move past delimmeter
          break;
        }
      }
      m.y_dist = atof(num_buf);
      //Serial.print("YDIST: ");
      //Serial.println(m.y_dist);
      for (int x = 0; x < num_pos; x++) num_buf[x] = '\0';

      /* Parse ZDIST */
      num_pos = 0;
      while(x < buff_pos)
      {
        num_buf[num_pos] = buff[x];
        num_pos++;
        x++;
        if (buff[x] == ':' || buff[x] == '|')
        {
          x++; //move past delimmeter
          break;
        }
      }
      m.z_dist = atof(num_buf);
      //Serial.print("ZDIST: ");
      //Serial.println(m.z_dist);
      for (int x = 0; x < num_pos; x++) num_buf[x] = '\0';

      /* Parse TARGET_VEL */
      num_pos = 0;
      while(x < buff_pos)
      {
        num_buf[num_pos] = buff[x];
        num_pos++;
        x++;
        if (buff[x] == ':' || buff[x] == '|')
        {
          x++; //move past delimmeter
          break;
        }
      }
      m.target_vel = atof(num_buf);
      //Serial.print("TARGET_VEL: ");
      //Serial.println(m.target_vel);
      for (int x = 0; x < num_pos; x++) num_buf[x] = '\0';


      m.x_step_count = abs(m.x_dist) * axis_scale[0];
      m.y_step_count = abs(m.y_dist) * axis_scale[1];
      m.z_step_count = abs(m.z_dist) * axis_scale[2];

      m.x_dir = true;
      m.y_dir = true;
      m.z_dir = true;
      if (m.x_dist < 0)
      {
        //Serial.println("X is CCW");
        m.x_dir = false;
      }
      else
      {
        //Serial.println("X is CW");
      }
      if (m.y_dist < 0)
      {
        //Serial.println("Y is CCW");
        m.y_dir = false;
      }
      else
      {
        //Serial.println("Y is CW");
      }
      if (m.z_dist < 0)
      {
        //Serial.println("Z is CCW");
        m.z_dir = false;
      }
      else
      {
        //Serial.println("Z is CW");
      }
      m.move_began_at = millis();
      m.reached_max_accel_at = 0;

      if (move_bottom_pos == 0)
      {
        //First move need to update dir pins because they only get updated on next_move()
        write_pin(X_DIR, m.x_dir);
        write_pin(Y_DIR, m.y_dir);
        write_pin(Z_DIR, m.z_dir);
        current_velocity = 0;
        m.vel_dir = true; //We know this move will be a velocity increase because there are no other moves on the stack
      }
      else
      {
          if (move_buffer[move_bottom_pos].target_vel > m.target_vel)
          {
            m.vel_dir = true;
            //Serial.println("Setting target velocity direction to +!");
          }
          else
          {
            m.vel_dir = false;
            //Serial.println("Setting target velocity direction to -!");
          }
      }
      m.reached_target_vel = false;
      move_buffer[move_bottom_pos++] = m; //Push move onto move bottom of move buffer!
    }
  }
}
void loop()
{
  if (Serial.available() > 0)
  {
    c = Serial.read();
    if (c == '\n')
    {
      //Serial.print("Buffer is: ");
      //Serial.println(buff);
      parseBuffer();
      for (int x = 0; x < buff_pos; x++)
      {
        buff[buff_pos] = '\0';
      }
      buff_pos = 0;
    }
    else
    {
      buff[buff_pos++] = c;
      //Serial.print("Added to buff: ");
      //Serial.print(c);
      //Serial.print(" at possition ");
      //Serial.println(buff_pos);
    }

  }
  for (int z = 0; z < 100; z++)
  {
    if (move_bottom_pos > 0) //There are motion events on the stack!
    {
      if (move_buffer[0].x_step_count == 0 && move_buffer[0].y_step_count == 0 && move_buffer[0].z_step_count == 0) //no more moves left, move on to next move
      {
        next_move();
        write_pin(X_DIR, move_buffer[0].x_dir);
        write_pin(Y_DIR, move_buffer[0].y_dir);
        write_pin(Z_DIR, move_buffer[0].z_dir);

        /*current_velocity = move_buffer[0].init_vel;
        Serial.print("Setting current velocity to init vel: ");
        Serial.println(current_velocity);
        feedrate_t rates = getIndividualAxisFeedrates(current_velocity, move_buffer[0].x_dist, move_buffer[0].y_dist, move_buffer[0].z_dist);
        if (rates.bad_calculation == false)
        {
          x_delay = rates.x;
          y_delay = rates.y;
          z_delay = rates.z;
        }*/
      }
      else
      {
        if (velocity_change_timer < millis() && move_buffer[0].reached_target_vel == false)
        {
          float calculated_velocity;
          feedrate_t rates;

          if (move_buffer[0].vel_dir == true)
          {
            //Increment velocity until we reach target velocity
            calculated_velocity = CalculateVelocity(current_velocity, millis() - move_buffer[0].move_began_at);
            current_velocity += (calculated_velocity - current_velocity);
            if (current_velocity > move_buffer[0].target_vel)
            {
              current_velocity = move_buffer[0].target_vel;
              move_buffer[0].reached_target_vel = true;
            }
            Serial.print("Updating Velocity+: ");
            Serial.println(current_velocity);
            rates = getIndividualAxisFeedrates(current_velocity, move_buffer[0].x_dist, move_buffer[0].y_dist, move_buffer[0].z_dist);
            if (rates.bad_calculation == false)
            {
              x_delay = rates.x;
              y_delay = rates.y;
              z_delay = rates.z;
            }
          }
          else
          {
            //Decrement velocity until we reach target velocity
            calculated_velocity = CalculateVelocity(current_velocity, millis() - move_buffer[0].move_began_at);
            current_velocity -= (calculated_velocity - current_velocity);
            if (current_velocity < move_buffer[0].target_vel)
            {
              current_velocity = move_buffer[0].target_vel;
              move_buffer[0].reached_target_vel = true;
            }
            Serial.print("Updating Velocity-: ");
            Serial.println(current_velocity);
            rates = getIndividualAxisFeedrates(current_velocity, move_buffer[0].x_dist, move_buffer[0].y_dist, move_buffer[0].z_dist);
            if (rates.bad_calculation == false)
            {
              x_delay = rates.x;
              y_delay = rates.y;
              z_delay = rates.z;
            }
          }
          velocity_change_timer = millis() + VELOCITY_UPDATE_PERIOD;
        }
        if (move_buffer[0].x_step_count > 0 && x_timer < micros())
        {
          //Serial.print("Moving X: ");
          //Serial.println(move_buffer[0].x_step_count);
          int latency = micros() - x_timer;
          //if (latency > 100) latency = 100;
          write_pin(X_STEP, HIGH);
          delayMicroseconds(STEP_LEN);
          write_pin(X_STEP, LOW);
          move_buffer[0].x_step_count--; //Decrement our step count
          //move_buffer[0].x_dist -= 1 / axis_scale[0];
          x_timer = micros() + x_delay - latency - STEP_LEN;
        }
        if (move_buffer[0].y_step_count > 0 && y_timer < micros())
        {
          //Serial.print("Moving Y: ");
          //Serial.println(move_buffer[0].y_step_count);
          int latency = micros() - y_timer;
          //if (latency > 100) latency = 100;
          write_pin(Y_STEP, HIGH);
          delayMicroseconds(STEP_LEN);
          write_pin(Y_STEP, LOW);
          move_buffer[0].y_step_count--;
          //move_buffer[0].y_dist -= 1 / axis_scale[1];
          y_timer = micros() + y_delay - latency - STEP_LEN;
        }
        if (move_buffer[0].z_step_count > 0 && z_timer < micros())
        {
          //Serial.print("Moving Z: ");
          //Serial.println(move_buffer[0].z_step_count);
          int latency = micros() - z_timer;
          //if (latency > 100) latency = 100;
          write_pin(Z_STEP, HIGH);
          delayMicroseconds(STEP_LEN);
          write_pin(Z_STEP, LOW);
          move_buffer[0].z_step_count--;
          //move_buffer[0].z_dist -= 1 / axis_scale[2];
          z_timer = micros() + z_delay - latency - STEP_LEN;
        }
      }
    }
  }
}
