#include "MqttClientExtended.h"


DeviceMqttClientExtended::DeviceMqttClientExtended() : DeviceMqttClient()
{
}

DeviceMqttClientExtended::~DeviceMqttClientExtended()
{
}

void DeviceMqttClientExtended::motionSensorStatePublish(char *sensorId, uint8_t state)
{
  Serial.printf("\nMQTT: Publish motion sensor state: %s\n", sensorId);
  publish(sensorId, 0, true, (state ? "1" : "0") );
}
