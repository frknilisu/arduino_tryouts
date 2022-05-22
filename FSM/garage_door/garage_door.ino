// Signal parameters
const uint8_t SW_CONTROLLER = 4;  // remote control signal
const uint8_t SW_LIMIT_OPEN = 2;  // limit switch for open door
const uint8_t SW_LIMIT_CLOSE = 3; // limit switch for closed door
 
const uint8_t SW_ACTIVE = LOW; // LOW -> INPUT_PULLUP, HIGH -> External pulldown
 
const uint8_t LED_TRANSITION = 7; // LED indicator we are in transition
const uint8_t LED_OPEN = 5; // LED indicator for open door
const uint8_t LED_CLOSE = 6; // LED indicator for closed door

class cSignal
{
  public:
    cSignal(uint8_t pin, uint8_t active): _pin(pin), _active(active) {}
 
    void begin(void) { pinMode(_pin, (_active == LOW) ? INPUT_PULLUP : INPUT); reset(); };
    void reset(void) { _state = WAIT_START; _sigLastActive = false; };

    bool isActive(void)
    {
      bool sigActive = (digitalRead(_pin) == _active);
      bool b = false;
      
      switch(_state)
      {
        case WAIT_START:
          if(!_sigLastActive && sigActive)
          {
            _state = DEBOUNCE;
            _sigLastActive = sigActive;
            _timeLastTransition = millis();
          }
          break;
        case DEBOUNCE:
          if(millis() - _timeLastTransition >= DEBOUNCE_TIME) 
          {
            _state = CHECK;
          }
          break;
        case CHECK:
          b = sigActive;
          _state = WAIT_END;
          break;
        case WAIT_END:
          if(!sigActive && _sigLastActive)
          {
            _sigLastActive = sigActive;
            _state = WAIT_START;
          }
          break;
        default:
          reset();
          break;
      }

      return b;
    }

  private:
    uint8_t _pin;     // pin for this signal
    uint8_t _active;  // HIGH or LOW
   
    const uint32_t DEBOUNCE_TIME = 50;
   
    enum { WAIT_START, DEBOUNCE, CHECK, WAIT_END } _state = WAIT_START;
    bool _sigLastActive = false;
    uint32_t _timeLastTransition = 0;
}

cSignal sigControl(SW_CONTROLLER, LOW);
cSignal sigLimitOpen(SW_LIMIT_OPEN, LOW);
cSignal sigLimitClose(SW_LIMIT_CLOSE, LOW);

void runMotor(int direction)
// direction parameter
// -1 runs the motor in direction to open the door
// 0 stops the motor
// 1 runs the motor in direction to close the door
{
  switch (direction)
  {
    case -1:  // opening door
      digitalWrite(LED_OPEN, HIGH);
      digitalWrite(LED_CLOSE, LOW);
      digitalWrite(LED_TRANSITION, HIGH);
      break;
 
    case 0:  // stop motion in all modes (motor off)
      digitalWrite(LED_TRANSITION, LOW);
      break;
 
    case 1:  // closing door
      digitalWrite(LED_CLOSE, HIGH);
      digitalWrite(LED_OPEN, LOW);
      digitalWrite(LED_TRANSITION, HIGH);
      break;
  }
}

void garageFSM()
{
  static enum { OPEN, OPEN2CLOSE, CLOSE, CLOSE2OPEN } state = OPEN;

  switch (state)
  {
    case OPEN:
      runMotor(0);
      if (sigControl.isActive())
        state = OPEN2CLOSE;
      break;
 
    case OPEN2CLOSE:
      runMotor(1);
      if (sigControl.isActive())
        state = CLOSE2OPEN;  // reverse the motion
      else if (sigLimitClose.isActive())
        state = CLOSE;       // reached the end of motion
      break;
 
    case CLOSE:
      runMotor(0);
      if (sigControl.isActive())
        state = CLOSE2OPEN;
      break;
 
    case CLOSE2OPEN:
      runMotor(-1);
      if (sigLimitOpen.isActive())
        state = OPEN;   // reached the end of motion
      break;
 
    default:
      state = CLOSE2OPEN;
      break;
  }
}

void setup() {
  // put your setup code here, to run once:
  // signal debouncers
  sigControl.begin();
  sigLimitOpen.begin();
  sigLimitClose.begin();
 
  // simulation LEDs
  pinMode(LED_TRANSITION, OUTPUT);
  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_CLOSE, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  garageFSM();
}
