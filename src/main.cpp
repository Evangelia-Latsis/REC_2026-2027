#include <Arduino.h>
//one thing to note HIGH -> no magnet detection, LOW -> magnetic detection
//each number represents a hall effect sensor connected to an ESP32 GPIO pin
const int sensorPins[] = {18, 19, 21, 22, 23}; 
const int numSensors = sizeof(sensorPins) / sizeof(sensorPins[0]);

const unsigned long debounceTime = 10; // 10 millisecond detection delay to avoid noise

int previousStates[5] = {HIGH, HIGH, HIGH, HIGH, HIGH}; //stores last raw sensor reading
int stableStates[5] = {HIGH, HIGH, HIGH, HIGH, HIGH}; //stores last confirmed sensor reading

unsigned long lastChangeTimes[5] = {0, 0, 0, 0, 0}; //records when each sensor's raw reading changed

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < numSensors; i++)
  {
    pinMode(sensorPins[i], INPUT_PULLUP);
  }

}

void loop() 
{
  unsigned long currentTime = millis();

  for (int i = 0; i < numSensors; i++)
  {
    int reading = digitalRead(sensorPins[i]);

    //Detect a change in the raw reading
    if (reading != previousStates[i])
    {
      
      previousStates[i] = reading;

      lastChangeTimes[i] = currentTime;
    }

    // check if the reading stabilized
    if (currentTime - lastChangeTimes[i] >= debounceTime)
    {

      if (reading != stableStates[i])
      {
        
        stableStates[i] = reading;

        //register only when a new magnet is detection
        if (stableStates[i] == LOW)
        {
          
          Serial.print("Sensor " + String(i + 1) + " detected at " + String(currentTime) + " ms");

        }
      }
    }
  }
}