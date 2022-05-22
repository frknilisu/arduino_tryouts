#define DEBUG 1
#if DEBUG
  #define FSM_STATE(s) { Serial.println(F("n:")); Serial.println(F(s)); }
  #define FSM_ERROR(s) { Serial.println(F("n:")); Serial.println(F(s)); }
#else
  #define FSM_STATE(s)
  #define FSM_ERROR(s)
#endif

#define SW_PIN 2
#define joySW 14
#define SW_ACTIVE LOW // LOW -> INPUT_PULLUP, HIGH -> EXTERNAL PULLDOWN

bool isPressed(bool reset = false) {
  const uint32_t DEBOUNCE_TIME = 50;
  static enum { WAIT_START, DEBOUNCE, CHECK, WAIT_END } state = WAIT_START;
  static uint32_t timeLastTransition = 0;
  static bool swLastActive = false;
  bool swActive = (analogRead(joySW) == SW_ACTIVE);
  bool b = false;

  if (reset) { 
    state = WAIT_START;
    swLastActive = false;
  }
  
  switch(state) 
  {
    case WAIT_START:   // waiting for start transition
      if (!swLastActive && swActive)   // transition to active
      {
        FSM_STATE("START");
        state = DEBOUNCE;
        swLastActive = swActive;       // save the current state
        timeLastTransition = millis(); // time from now
      }
      break;
 
    case DEBOUNCE:  // wait for the debounce time
      if (millis() - timeLastTransition >= DEBOUNCE_TIME)
      {
        FSM_STATE("DEBOUNCE");
        state = CHECK;
      }
      break;
 
    case CHECK:    // check switch still active
      FSM_STATE("CHECK");
      b = swActive;   // return status
      state = WAIT_END;
      break;
 
     case WAIT_END:
       if (!swActive && swLastActive)   // transition to inactive
       {
          FSM_STATE("WAIT_END");
          swLastActive = swActive;
          state = WAIT_START;
       }
       break;
 
    default:
      state = WAIT_START;
      break;
  }
  return b;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(joySW, (SW_ACTIVE == LOW) ? INPUT_PULLUP: INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(isPressed()) {
    Serial.println("\n--> PRESS");
  }
}
