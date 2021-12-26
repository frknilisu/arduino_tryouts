#define joyX 2
#define joyY 4
#define joySW 14

volatile byte pressed = false;

int xValue = 0; // variable to store x value
int yValue = 0; // variable to store y value
int notPressed = 0; // variable to store the button's state => 1 if not 
int xMap, yMap;


void setup() {
  Serial.begin(115200); // Setup serial connection for print out to console
  
  pinMode(joyX, INPUT); // setup x input
  pinMode(joyY, INPUT); // setup y input
  pinMode(joySW, INPUT_PULLUP); // we use a pullup-resistor for the button functionality
}
 
void loop() {
  // put your main code here, to run repeatedly:
  xValue = analogRead(joyX);  // reading x value [range 0 -> 1023]
  yValue = analogRead(joyY);  // reading y value [range 0 -> 1023]
  notPressed = digitalRead(joySW); // reading button state: 1 = not pressed, 0 = pressed
  
  xMap = map(xValue, 0, 1023, 0, 500);
  yMap = map(yValue, 0, 1023, 0, 500);
 
  //print the values with to plot or view
  Serial.print(xValue);
  Serial.print(", ");
  Serial.print(yValue);
  Serial.print("\t");
  Serial.print(xMap);
  Serial.print(", ");
  Serial.print(yMap);
  Serial.print(", Not pressed: ");
  Serial.println(notPressed);
  Serial.println();

  delay(500);
}
