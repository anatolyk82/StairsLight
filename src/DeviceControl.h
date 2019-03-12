#ifndef ESP_LIGHT_DEVICE_CONTROL_H
#define ESP_LIGHT_DEVICE_CONTROL_H

#include <map>
#include <FastLED.h>
#include "Config.h"

#define LED_TYPE    WS2812
#define COLOR_ORDER GRB

struct ChaosEffectHelper {
  uint8_t hue;
  uint8_t saturation;
  uint8_t brightness;
  uint8_t maxBrightness;
  bool up;
};

/*
 * This class may differ from one device to another
 * depending on how the device is controled.
 */
class DeviceControl
{
public:
  DeviceControl();
  ~DeviceControl();

  /*
   * Initialize the device.
   * The function must be called once in setup()
   */
  void initDevice();

  /*
   * Set a reference to the device state structure.
   * This must be called in setup() before sending/receiving any data.
   */
  void setDeviceStateReference(struct DeviceState *deviceState) {
    m_deviceState = deviceState;
  }

  /*
   * The function applies all changes from
   * from the device state structure
   */
  void updateDeviceState();

  /*
   * This function sets a reference to a callback to be called
   * when the current state of the device has changed.
   */
  void onDeviceStateChanged(std::function<void(void)> sendMessageState) {
    m_sendMessageState = sendMessageState;
  }

  /*
   * Run the device.
   * It must be called in loop()
   */
  void run();

  /* Effects */
  void efLightUp();
  void efLightDown();
  void efDimUp();
  void efDimDown();
  void efStartLight();
  void efEndLight();
  void efNightLight();
  void efChaos();
  void efCometUp();
  void efCometDown();

private:
  void transition();
  float m_current_brightness;
  bool m_inTransition;

  std::function<void(void)> m_sendMessageState = nullptr;

  void setColor(byte red, byte green, byte blue, byte brightness);

  CRGB leds[NUM_LEDS];
  struct DeviceState *m_deviceState;

  /* Effects */
  uint8_t m_effect_lightUp_index = NUM_LEDS - 1;
  uint8_t m_effect_lightDown_index = 0;

  ChaosEffectHelper chaosEffectHelper[NUM_LEDS];

  std::map< std::string, std::function<void(void)> > m_lightEffectsList = {
    {"LightDown",      std::bind(&DeviceControl::efLightDown, this)},
    {"LightUp",        std::bind(&DeviceControl::efLightUp, this)},
    {"DimDown",        std::bind(&DeviceControl::efDimDown, this)},
    {"DimUp",          std::bind(&DeviceControl::efDimUp, this)},
    {"StartLight",     std::bind(&DeviceControl::efStartLight, this)},
    {"EndLight",       std::bind(&DeviceControl::efEndLight, this)},
    {"NightLight",     std::bind(&DeviceControl::efNightLight, this)},
    {"Chaos",          std::bind(&DeviceControl::efChaos, this)},
    {"CometUp",        std::bind(&DeviceControl::efCometUp, this)},
    {"CometDown",      std::bind(&DeviceControl::efCometDown, this)},
  };
};

#endif //ESP_LIGHT_DEVICE_CONTROL_H
