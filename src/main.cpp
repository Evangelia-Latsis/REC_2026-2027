#include <Arduino.h>
//one thing to note HIGH -> no magnet detection, LOW -> magnetic detection
//each number represents a hall effect sensor connected to an ESP32 GPIO pin
const int sensorPins[] = {18, 19, 21, 22, 23}; 
const int numSensors = sizeof(sensorPins) / sizeof(sensorPins[0]);

const unsigned long debounceTime = 10; // 10 millisecond detection delay to avoid noise


int previousStates[numSensors]; //stores last raw sensor reading
int stableStates[numSensors]; //stores last confirmed sensor reading
unsigned long lastChangeTimes[numSensors]; //records when each sensor's raw reading changed

struct SensorEvent 
{
  int sensorID;

  unsigned long timestamp;
};

const int MAX_EVENTS = 50;
SensorEvent events[MAX_EVENTS];
int nextEvent = 0;

void setup() {
  Serial.begin(115200);

  //initialize all the sensors
  for (int i = 0; i < numSensors; i++)
  {

    pinMode(sensorPins[i], INPUT_PULLUP);
    
    int initialState = digitalRead(sensorPins[i]);

    previousStates[i] = initialState;
    stableStates[i] = initialState;

    lastChangeTimes[i] = millis();
  }

  Serial.println("Tracking has started");

}

void recordEvent(int sensorIndex, unsigned long timestamp)
{

  //store our event
  events[nextEvent].sensorID = sensorIndex + 1;

  events[nextEvent].timestamp = timestamp;

  //move to the next storage location, wrapping around if we readch the end of the array
  nextEvent = (nextEvent + 1) % MAX_EVENTS;

  Serial.print("Sensor " + String(sensorIndex + 1) + " detected at " + String(timestamp) + " ms");
}

void loop() 
{
  unsigned long currentTime = millis();

  //loop through all the sensors
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

        //record new detection
        if (stableStates[i] == LOW)
        {
          
          recordEvent(i, currentTime);

        }
      }
    }
  }
}