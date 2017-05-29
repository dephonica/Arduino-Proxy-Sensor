#include "proxysensor.h"

ProxySensor *sensor;

bool lampSwitch = false;

void setup() 
{
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  
  Serial.begin(115200);

  sensor = new ProxySensor();
}

void loop() 
{
  if (isSwitched[0])
  {
    isSwitched[0] = false;
    Serial.println(isKeyPressed[0] ? "yes" : "no");

    if (isKeyPressed[0] == false)
    {
      lampSwitch = !lampSwitch;
      digitalWrite(2, lampSwitch);
    }
  }
}

