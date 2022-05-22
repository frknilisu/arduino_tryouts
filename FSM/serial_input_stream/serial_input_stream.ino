#define DEBUG 0
#if DEBUG
  #define FSM_STATE(s) { Serial.print(F("n:")); Serial.print(F(s)); Serial.print(F(": ")); Serial.print(c); }
  #define FSM_MESG(s) { Serial.print(F("n->")); Serial.print(F(s)); Serial.print(F(": ")); Serial.print(cBuf); }
#else
  #define FSM_STATE(s)
  #define FSM_MESG(s)
#endif
  
bool processInput(bool reset = false)
{
  const uint32_t TIMEOUT_DELAY = 1000;
  const uint16_t MAX_LEN = 20;
  const char CH_START = '<';   const char CH_END = '>';
 
  static char cBuf[MAX_LEN+1] = "";
  static uint16_t lenBuf = 0;
  static enum { WAIT_START, BUFFERING, PROCESS_MESG } state = WAIT_START;
  static uint32_t timeLastChar = 0;
  char c = '\0';
  bool b = false;
 
  if (reset) state = WAIT_START;
  // read the next char if there is one
  // don't read if we are about to process the message
  if (state != PROCESS_MESG)
  {
    if (Serial.available())
    {
      c = Serial.read();
      timeLastChar = millis();
    }
  }
  
  // process the char based on state
  switch (state)
  {
    case WAIT_START:   // waiting for start character
      if (c != '\0')
        FSM_STATE("START");
      if (c == CH_START)
      {
         state = BUFFERING;
         lenBuf = 0;
         memset(cBuf, '\0', sizeof(char)*(MAX_LEN+1));  // clear the buffer
      }
      break;
 
    case BUFFERING:   // buffer up all the characters
      if (c != '\0')
        FSM_STATE("BUFFER"); 
 
      // have we timed out?
      if (millis() - timeLastChar >= TIMEOUT_DELAY)
      { 
        FSM_MESG("Timeout");
        state = WAIT_START;
        break;
      }
   
      // process the character
      if (c == CH_END)
        state = PROCESS_MESG;
      else if (c != '\0')   // not empty
      {
        cBuf[lenBuf++] = c;
        if (lenBuf == MAX_LEN)   // buffer overflow
        {
          state = WAIT_START;
          FSM_MESG("Overflow");
        }
      }
      break;
 
    case PROCESS_MESG:     // do something with the message
      FSM_STATE("PROCESS_MESG");
      FSM_MESG("Received");
      state = WAIT_START;
      b = true;
      break;
 
    default:
      state = WAIT_START;
      break;
   }
 return(b);
 }
 
void setup(void) 
{
  Serial.begin(115200);
}
 
void loop(void) 
{
  if (processInput())
    Serial.print("\nYIPPEE");
  // do something else
}
