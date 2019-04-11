#include <AccelStepper.h>
#include <Machine.h>

//AccelStepper Xaxis(1, 2, 5); // pin 2 = step, pin 5 = direction
//AccelStepper Yaxis(1, 3, 6); // pin 3 = step, pin 6 = direction
//AccelStepper Zaxis(1, 4, 7); // pin 4 = step, pin 7 = direction

AccelStepper Xaxis(1, X_STEP, X_DIR); // pin 3 = step, pin 6 = direction
AccelStepper Yaxis(1, Y_STEP, Y_DIR); // pin 4 = step, pin 7 = direction

void setup() {
  Xaxis.setMaxSpeed(X_SCALE * 0.666);
  Xaxis.setAcceleration(X_SCALE * X_ACCEL);
  Xaxis.moveTo(X_SCALE * 10);

  Yaxis.setMaxSpeed(Y_SCALE * 0.666);
  Yaxis.setAcceleration(Y_SCALE * Y_ACCEL);
  Yaxis.moveTo(Y_SCALE * 10);
}

void loop() {
   Xaxis.run();
   Yaxis.run();
}
