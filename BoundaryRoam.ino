/* BoundaryRoam.ino
 *
 * Drives the Zumo robot around within a region marked by boundary lines
 * detected by the onboard Zumo reflectance sensor array.
 *
 * The ZumoReflectanceSensorArray is initialised with its default pinout so the
 * library controls the LEDON pin, pulsing the IR emitter LEDs (and the visible
 * red indicator LEDs) only during sensor reads.  Requires the LEDON shorting
 * block on the sensor array PCB to be installed:
 *   - Arduino Uno / older Arduino  → connect LEDON to pin 2
 *   - Arduino Leonardo / A-Star 32U4 → connect LEDON to A4
 *
 * When a boundary line is detected, the robot executes an avoidance sequence:
 *   1. Reverse away from the line
 *   2. Turn to face a new direction
 *   3. Drive forward for a few seconds (restarting avoidance if another line appears)
 *   4. Pause briefly
 *   5. Turn to a fresh random direction, then resume roaming
 *
 * Press the button to start (3-beep countdown). Press again to pause/restart.
 */

#include <Wire.h>
#include <ZumoShield.h>

#define LED_PIN 13

Pushbutton button(ZUMO_BUTTON);

// Reflectance sensor settings
// The sensor array PCB traces for A2 (sensor 1) and A3 (sensor 4) have been
// cut; those pins are reserved for forward and rear range detectors.
// The remaining 4 downward-facing sensors, left-to-right, are on pins:
//   index 0 → pin 5   (leftmost)
//   index 1 → pin A0  (left-center)
//   index 2 → pin 11  (right-center)
//   index 3 → pin 4   (rightmost)
// LEDON shorting block must be installed: pin 2 (Uno) or A4 (Leonardo/32U4).
#define NUM_SENSORS   4
unsigned int sensor_values[NUM_SENSORS];
#define QTR_THRESHOLD  1500  // microseconds; tune for your surface/lighting
static const byte SENSOR_PINS[NUM_SENSORS] = {5, A0, 11, 4};
ZumoReflectanceSensorArray sensors;

// Motor speeds (0-400)
ZumoMotors motors;
#define ROAM_SPEED     200
#define REVERSE_SPEED  200
#define TURN_SPEED     200

// Phase durations (milliseconds)
#define REVERSE_DURATION   300
#define TURN_MIN_DURATION  300   // minimum turn time
#define TURN_RAND_EXTRA    300   // adds 0-299 ms of randomness to turns
#define TURN_180_DURATION  650   // approximate 180° spin on center-sensor detection
#define DRIVE_DURATION    6000   // drive forward after avoidance sequence
#define PAUSE_DURATION     900   // pause before the second turn

#define RIGHT   1
#define LEFT   -1

ZumoBuzzer buzzer;

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------
enum State { ROAMING, REVERSING, TURNING_1, DRIVING, PAUSING, TURNING_2 };
State current_state;
unsigned long state_start;
unsigned long state_duration;
int turn_direction;
bool needs_180;

void enterState(State s, unsigned long duration = 0)
{
  current_state  = s;
  state_start    = millis();
  state_duration = duration;
}

bool elapsed()
{
  return (millis() - state_start) >= state_duration;
}

// ---------------------------------------------------------------------------
// Returns true when any boundary line is detected and sets dir to the turn
// direction that would steer the robot away from the detected edge.
// ---------------------------------------------------------------------------
bool lineDetected(int &dir, bool &spin180)
{
  sensors.read(sensor_values);

  // sensor_values[0] = leftmost  (pin 5)   — left edge  → turn RIGHT
  // sensor_values[1] = left-center (pin A0) — center    → reverse + 180°
  // sensor_values[2] = right-center (pin 11) — center   → reverse + 180°
  // sensor_values[3] = rightmost (pin 4)   — right edge → turn LEFT
  bool outer_left  = (sensor_values[0] > QTR_THRESHOLD);
  bool outer_right = (sensor_values[3] > QTR_THRESHOLD);
  bool center      = (sensor_values[1] > QTR_THRESHOLD || sensor_values[2] > QTR_THRESHOLD);

  if (!outer_left && !outer_right && !center) return false;

  // Center-only detection: reverse and spin 180°
  spin180 = center && !outer_left && !outer_right;

  if      (outer_left  && !outer_right) dir = RIGHT;  // edge on left  → turn right
  else if (outer_right && !outer_left)  dir = LEFT;   // edge on right → turn left
  else                                  dir = (random(2) == 0) ? RIGHT : LEFT;

  return true;
}

// ---------------------------------------------------------------------------
// Wait for a button press then play a 3-beep countdown.
// ---------------------------------------------------------------------------
void waitForStart()
{
  digitalWrite(LED_PIN, HIGH);
  button.waitForButton();
  digitalWrite(LED_PIN, LOW);

  for (int i = 0; i < 3; i++)
  {
    delay(1000);
    buzzer.playNote(NOTE_G(3), 50, 15);
  }
  delay(1000);
}

// ---------------------------------------------------------------------------
void setup()
{
  pinMode(LED_PIN, OUTPUT);
  randomSeed((unsigned int) millis());
  buzzer.playMode(PLAY_AUTOMATIC);
  sensors.init(SENSOR_PINS, NUM_SENSORS);

//   motors.flipLeftMotor(true);
//   motors.flipRightMotor(true);

  waitForStart();

  motors.setSpeeds(ROAM_SPEED, ROAM_SPEED);
  enterState(ROAMING);
}

// ---------------------------------------------------------------------------
void loop()
{
  // Pause/restart on button press
  if (button.isPressed())
  {
    motors.setSpeeds(0, 0);
    button.waitForRelease();
    waitForStart();
    motors.setSpeeds(ROAM_SPEED, ROAM_SPEED);
    enterState(ROAMING);
    return;
  }

  switch (current_state)
  {
    // ---- Roaming: drive straight, watching for boundary lines ----
    case ROAMING:
      if (lineDetected(turn_direction, needs_180))
      {
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        enterState(REVERSING, REVERSE_DURATION);
      }
      break;

    // ---- Reversing: back away from the detected line ----
    case REVERSING:
      if (elapsed())
      {
        unsigned long turn_dur = needs_180
          ? TURN_180_DURATION
          : TURN_MIN_DURATION + (unsigned long)random(TURN_RAND_EXTRA);
        motors.setSpeeds(TURN_SPEED * turn_direction, -TURN_SPEED * turn_direction);
        enterState(TURNING_1, turn_dur);
      }
      break;

    // ---- First turn: rotate to face away from the boundary ----
    case TURNING_1:
      if (elapsed())
      {
        motors.setSpeeds(ROAM_SPEED, ROAM_SPEED);
        enterState(DRIVING, DRIVE_DURATION);
      }
      break;

    // ---- Drive: move forward for several seconds; restart avoidance if ----
    // ---- another line appears before the timer expires               ----
    case DRIVING:
      if (lineDetected(turn_direction, needs_180))
      {
        motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
        enterState(REVERSING, REVERSE_DURATION);
      }
      else if (elapsed())
      {
        motors.setSpeeds(0, 0);
        enterState(PAUSING, PAUSE_DURATION);
      }
      break;

    // ---- Pause: brief stop before changing direction ----
    case PAUSING:
      if (elapsed())
      {
        turn_direction = (random(2) == 0) ? RIGHT : LEFT;
        motors.setSpeeds(TURN_SPEED * turn_direction, -TURN_SPEED * turn_direction);
        enterState(TURNING_2, TURN_MIN_DURATION + random(TURN_RAND_EXTRA));
      }
      break;

    // ---- Second turn: pick a fresh direction then return to roaming ----
    case TURNING_2:
      if (elapsed())
      {
        motors.setSpeeds(ROAM_SPEED, ROAM_SPEED);
        enterState(ROAMING);
      }
      break;
  }
}
