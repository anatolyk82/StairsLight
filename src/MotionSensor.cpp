#include "MotionSensor.h"
#include <ESP8266WiFi.h>

MotionSensor::MotionSensor(char* sensorId, uint8_t sensorGpio) : 
  m_sensorGpio(sensorGpio),
  m_motionState(0)
{
  strncpy(m_sensorId, sensorId, 64);
  pinMode(m_sensorGpio, INPUT);
}


MotionSensor::~MotionSensor()
{
}

void MotionSensor::run()
{
  uint8_t pinValue = digitalRead(m_sensorGpio);
  if (pinValue != m_motionState) {
    m_motionState = pinValue;
    if (m_updateMotionState) {
      m_updateMotionState(m_sensorId, m_motionState);
    }
  }
}
