#define DIR_IN 2
#define DIR_OUT 6
#define STEP_IN 3
#define STEP_OUT 7

#define TH_UP A0
#define TH_DOWN A1
#define ARC_OK A2
#define THC_ENABLE A3
#define TORCH_ON A4
#define COMP_SPEED A5

#define AXIS_DOWN LOW
#define AXIS_UP HIGH

#define COMP_DELAY_MIN 30000 //uS
#define COMP_DELAY_MAX 10 //uS

void step_interupt_high(){
  write_pin(STEP_OUT, HIGH);
  delayMicroseconds(10);
  write_pin(STEP_OUT, LOW);
}
void write_pin(byte pin, bool value)
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
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(DIR_IN, INPUT);
  

  pinMode(STEP_IN, INPUT);
  attachInterrupt(digitalPinToInterrupt(STEP_IN), step_interupt_high, RISING);

  pinMode(STEP_OUT, OUTPUT);
  pinMode(DIR_OUT, OUTPUT);


  pinMode(12, OUTPUT);

  pinMode(TH_UP, INPUT_PULLUP);
  pinMode(TH_DOWN, INPUT_PULLUP);
  pinMode(ARC_OK, INPUT_PULLUP);
  //pinMode(TORCH_ON, INPUT_PULLUP);
  pinMode(THC_ENABLE, INPUT_PULLUP);
  
}
/*void dir_interupt(){
  dir_register = !dir_register;
  digitalWrite(DIR_OUT, dir_register);
}*/

unsigned long timer = 0;
void loop()
{
  //Serial.print("ARC_OK: ");
  //Serial.println(digitalRead(ARC_OK));
  //Serial.print("TORCH_ON: ");
  //Serial.println(analogRead(TORCH_ON));
  //Serial.print("THC_ENABLE: ");
  //Serial.println(digitalRead(THC_ENABLE));
  if (digitalRead(ARC_OK) == LOW && analogRead(TORCH_ON) > 300 && digitalRead(THC_ENABLE) == HIGH)
  {
    int comp_delay = map(analogRead(COMP_SPEED),0, 1024, COMP_DELAY_MIN, COMP_DELAY_MAX);
    if (digitalRead(TH_UP) == LOW)
    {
      if (timer < micros())
      {
        write_pin(DIR_OUT, AXIS_UP);
        write_pin(STEP_OUT, HIGH);
        delayMicroseconds(3);
        write_pin(STEP_OUT, LOW);
        timer = micros() + comp_delay;
      }
    }
    if (digitalRead(TH_DOWN) == LOW)
    {
      if (timer < micros())
      {
        write_pin(DIR_OUT, AXIS_DOWN);
        write_pin(STEP_OUT, HIGH);
        delayMicroseconds(3);
        write_pin(STEP_OUT, LOW);
        timer = micros() + comp_delay;
      }
    }
  }
  else
  {
    write_pin(DIR_OUT, digitalRead(DIR_IN));
    delay(10);
  }
  
}