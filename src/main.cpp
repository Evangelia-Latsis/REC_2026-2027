#include <Arduino.h>
#include <math.h>
//one thing to note HIGH -> no magnet detection, LOW -> magnetic detection
//each number represents a hall effect sensor connected to an ESP32 GPIO pin
const int sensorPins[] = {18, 19, 21, 22, 23};
const float sensorPositions[] = {0.0, 18.0, 35.0, 51.0, 70.0}; // [unit: cms] this is arbitrary we're gonna have to measure the actual distance later 
const int numSensors = sizeof(sensorPins) / sizeof(sensorPins[0]);

const float trackLength = 100; // length of entire track, I have it in centimeters right now

const float train1StartPosition = 0.0;    // Station
const float train2StartPosition = 75;  // Transfer queue

const unsigned long debounceTime = 10; // 10 millisecond detection delay to avoid noise


int previousStates[numSensors]; //stores last raw sensor reading
int stableStates[numSensors]; //stores last confirmed sensor reading
unsigned long lastChangeTimes[numSensors]; //records when each sensor's raw reading changed

//event storage
struct SensorEvent 
{
  int sensorID;

  unsigned long timestamp;
};

const int MAX_EVENTS = 50;
SensorEvent events[MAX_EVENTS];
int nextEvent = 0;

//for coaster tracking
struct Coaster 
{
  float positon;
  float velocity;

  unsigned long lastDetectionTime;

  int lastSensor;
  int nextSensor;

  bool active;
  bool hasPreviousDetection;
  bool velocityValid;
};

Coaster coasters[2];

int findNextSensor(float position)
{
  for (int i = 0; i < numSensors; i++)
  {
    if (sensorPositions[i] > position)
    {
      return i;
    }
  }

  return 0;
}

float calculateDistance(float previousPosition, float currentPosition)
{

  float distance = currentPosition - previousPosition;

  if (distance < 0)
  {
    distance += tracLength;
  }

  return distance;
}

flaot predictedPosition(int coasterID)
{
  Coaster &coaster = coasters[coasterID];

  if (!coaster.velocityValid)
  {
    return coaster.position;
  }

  unsigned long currentTime = millis();

  float elapsedSeconds = (currentTime - coaster.lastDetectionTime) / 1000.0;

   float predictedPos = coaster.position + coaster.velocity * elapsedSeconds; //x(t) = x0 + v(t -t0)
  // keep the positioon inside the track length
  float predictedPos = fmod(predictedPosition, trackLength);

  if (predictedPos < 0)
  {
    predictedPos +=trackLength;
  }

  return predictedPos;
}

void initializeCoasters()
{
 coaster[0].position = train1StartPosition;

 coaster[1].position = train2StartPosition;

  for (int i = 0; i < 2; i++) 
  {

    coasters[i].velocity = 0.0;

    coasters[i].lastDetectionTime = millis();

    coasters[i].lastSensor = -1;

    // Determine which sensor each train should encounter first after its starting position.
    coasters[i].nextSensor = findNextSensor(coasters[i].position);

    coasters[i].active = true;

    coasters[i].hasPreviousDetection = false;

    coasters[i].velocityValid = false;
  }
}

void recordEvent(int sensorIndex, unsigned long timestamp)
{

  //store our event
  events[nextEvent].sensorID = sensorIndex + 1;

  events[nextEvent].timestamp = timestamp;

  //move to the next storage location, wrapping around if we readch the end of the array
  nextEvent = (nextEvent + 1) % MAX_EVENTS;

  Serial.println("Sensor " + String(sensorIndex + 1) + " detected at " + String(timestamp) + " ms");


}

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