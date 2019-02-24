#ifndef ESP_MOTION_SENSOR_H
#define ESP_MOTION_SENSOR_H

#include <functional>

class MotionSensor
{
public:
  MotionSensor(char* sensorId, uint8_t sensorGpio);
  ~MotionSensor();

  /*
   * The function sets a callback to be called when 
   * the state of motion sensor changes.
   */
   void onMotionStateChanged(std::function<void(char*, uint8_t)> updateMotionState){
    m_updateMotionState = updateMotionState;
  }

  /* 
   * Run the device.
   * It must be called in loop()
   */
  void run();

  /*
   * It returns the sensor id.
   */
  const char* sensorId() const {
    return m_sensorId;
  }

private:
  uint8_t m_sensorGpio;
  uint8_t m_motionState;
  std::function<void(char*, uint8_t)> m_updateMotionState;
  char m_sensorId[64];
};

#endif //ESP_MOTION_SENSOR_H
