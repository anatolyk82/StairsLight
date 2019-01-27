
#include <SimpleTimer.h>          // https://github.com/schinken/SimpleTimer

#include "Config.h"
#include "UiManager.h"
#include "MqttClientExtended.h"
#include "DeviceControl.h"
#include "MotionSensor.h"

/* Create a UI manager */
UiManager uiManager;

/* MQTT Client instance */
DeviceMqttClientExtended mqttClient;

/* The state of this device */
DeviceState deviceState;

/* Device control object */
DeviceControl device;

/* Timer to publish the current state */
SimpleTimer timer;

/* Motion sensors */
MotionSensor motionSensor1(MOTION_SENSOR1_TOPIC, MOTION_SENSOR1_PIN);
MotionSensor motionSensor2(MOTION_SENSOR2_TOPIC, MOTION_SENSOR2_PIN);

void publishDeviceStateTimer() {
  mqttClient.publishDeviceState();
}

void setup() {
 
  /* Init serial port */
  Serial.begin(115200);
  Serial.println();

  //clean FS, for testing
  //SPIFFS.format();

  /* Create UI and connect to WiFi */
  uiManager.initUIManager(false);

  /* Configure MQTT */
  mqttClient.setDeviceStateReference( &deviceState );
  int p = atoi( uiManager.mqttPort() );
  mqttClient.setServer( uiManager.mqttServer(), p );
  mqttClient.setCredentials( uiManager.mqttLogin(), uiManager.mqttPassword() );
  mqttClient.setKeepAlive( MQTT_KEEP_ALIVE_SECONDS );
  mqttClient.setWill( MQTT_TOPIC_STATUS, 1, true, MQTT_STATUS_PAYLOAD_OFF ); //topic, QoS, retain, payload
  String string_client_id( uiManager.mqttClientId() );
  string_client_id.trim();
  if (string_client_id != String("")) {
    mqttClient.setClientId( uiManager.mqttClientId() );
  }

  /* Set a callback to update the actual state of the device when an mqtt command is received */
  mqttClient.onDeviceStateUpdate( std::bind(&DeviceControl::updateDeviceState, &device) );

  /* Connect the MQTT client to the broker */
  int8_t attemptToConnectToMQTT = 0;
  Serial.println("MQTT: Connect to the broker");
  while ( (mqttClient.connected() == false) && (attemptToConnectToMQTT < 5) ) {
    if (WiFi.isConnected()) {
      Serial.printf("MQTT: Attempt %d. Connecting to broker [%s:%d]: login: [%s] password: [%s] ... \n", attemptToConnectToMQTT, uiManager.mqttServer(), p, uiManager.mqttLogin(), uiManager.mqttPassword() );
      mqttClient.connect();
    } else {
      //attemptToConnectToMQTT = 0;
      Serial.println("MQTT: WiFi is not connected. Try to reconnect WiFi.");
      WiFi.reconnect();
    }
    delay(3000);
    attemptToConnectToMQTT += 1;
  }

  /* Set callbacks for motion sensors */
  motionSensor1.onMotionStateChanged( std::bind(&DeviceMqttClientExtended::motionSensorStatePublish, &mqttClient, std::placeholders::_1, std::placeholders::_2) );
  motionSensor2.onMotionStateChanged( std::bind(&DeviceMqttClientExtended::motionSensorStatePublish, &mqttClient, std::placeholders::_1, std::placeholders::_2) );

  /* If there is still no connection here, restart the device */  
  if (!WiFi.isConnected()) {
    Serial.println("setup(): WiFi is not connected. Reset the device to initiate connection again.");
    ESP.restart();
  }

  if (!mqttClient.connected()) {
    Serial.println("setup(): The device is not connected to MQTT. Reset the device to initiate connection again.");
    ESP.restart();
  }

  /* Initialize the device */
  device.setDeviceStateReference( &deviceState );
  device.initDevice();

  /* Publish device state periodicly */
  timer.setInterval(INTERVAL_PUBLISH_STATE, publishDeviceStateTimer);
}


void loop() {
  timer.run();
  device.run();

  motionSensor1.run();
  motionSensor2.run();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("loop(): WiFi is not connected. Reset the device to initiate connection again.");
    ESP.restart();
  }
}
