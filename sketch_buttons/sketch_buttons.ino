//#define IS_SLAVE

#define BUTTON_IN 2
#define SIGNAL_OUT 3
#define SIGNAL_IN 5

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_IN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

#if defined(IS_SLAVE)
  pinMode(SIGNAL_IN, INPUT);
#else
  pinMode(SIGNAL_OUT, OUTPUT);
#endif

}

void loop() {

#if defined(IS_SLAVE)
  int buttonState = digitalRead(SIGNAL_IN);
#else
  int buttonState = digitalRead(BUTTON_IN);
#endif
  //Serial.println(buttonState);

  if (buttonState == HIGH) {
    digitalWrite(LED_BUILTIN, HIGH);

#if !defined(IS_SLAVE)
    digitalWrite(SIGNAL_OUT, HIGH);
#endif
  } else {
    // turn LED off:
    digitalWrite(LED_BUILTIN, LOW);
#if !defined(IS_SLAVE)
    digitalWrite(SIGNAL_OUT, LOW);
#endif
  }
  
  //digitalWrite(LED_BUILTIN, HIGH);
}
