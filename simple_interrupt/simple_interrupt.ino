#define joyX 2
#define joyY 4
#define joySW 14

volatile bool joyPressed = false;

volatile int interruptCounter = 0;
int numberOfInterrupts = 0;
 
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void handleInterrupt() {
  noInterrupts();
  portENTER_CRITICAL_ISR(&mux);
  interruptCounter++;
  joyPressed = true;
  portEXIT_CRITICAL_ISR(&mux);
  interrupts();
}

void setup() {
  Serial.begin(115200); // Setup serial connection for print out to console
  Serial.println("Monitoring interrupts: ");
  pinMode(joySW, INPUT_PULLUP); // we use a pullup-resistor for the button functionality
  digitalWrite(joySW, HIGH);  // internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(joySW), handleInterrupt, FALLING);  // attach interrupt handler
}

void loop() {
  /*
  if(interruptCounter>0){
      portENTER_CRITICAL(&mux);
      interruptCounter--;
      
      portEXIT_CRITICAL(&mux);
 
      numberOfInterrupts++;
      Serial.print("An interrupt has occurred. Total: ");
      Serial.println(numberOfInterrupts);
  }*/

  if(joyPressed){
    Serial.println("Joystick button has been pressed");
    joyPressed = false;
  }

  // show loop is running
  //Serial.println("Loop is running");
  //delay(2000);
}
