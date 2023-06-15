#define AUDIO_IN A0
#define BUTTON_IN 2

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_IN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {


  int buttonState = digitalRead(BUTTON_IN);
  //Serial.println(buttonState);

  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    // turn LED off:
    digitalWrite(LED_BUILTIN, LOW);
  }


  int sensorValue = analogRead(AUDIO_IN);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  Serial.println(voltage);
}
