#ifndef ESP_LIGHT_MQTT_CLIENT_EXTENDED_H
#define ESP_LIGHT_MQTT_CLIENT_EXTENDED_H

#include <AsyncMqttClient.h>      // https://github.com/marvinroger/async-mqtt-client + (https://github.com/me-no-dev/ESPAsyncTCP)
#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson (ver: 5.x)
#include "MqttClient.h"

#include <functional>

class DeviceMqttClientExtended : public DeviceMqttClient
{
public:
  DeviceMqttClientExtended();
  ~DeviceMqttClientExtended();

  void motionSensorStatePublish(char *sensorId, uint8_t state);
};

#endif //ESP_LIGHT_MQTT_CLIENT_EXTENDED_H
