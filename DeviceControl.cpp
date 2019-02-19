#include "DeviceControl.h"
#include "Config.h"

DeviceControl::DeviceControl() :
  m_deviceState(nullptr),
  m_inTransition(false)
{
}


DeviceControl::~DeviceControl()
{
}


void DeviceControl::initDevice()
{
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
}

void DeviceControl::setColor(byte red, byte green, byte blue, byte brightness)
{
  byte realRed = map(red, 0, 255, 0, brightness);
  byte realGreen = map(green, 0, 255, 0, brightness);
  byte realBlue = map(blue, 0, 255, 0, brightness);
  fill_solid( leds, NUM_LEDS, CRGB(realRed, realGreen, realBlue) );
}

void DeviceControl::updateDeviceState()
{
  Serial.printf("DeviceControl: Update state. Transition: %d\n", m_deviceState->transition);

  if (m_deviceState->transition != 0) {
    m_current_brightness = m_deviceState->state ? 0 : m_deviceState->brightness;
    m_inTransition = true;
    return; 
  }

  if (m_deviceState->state) {
    Serial.printf("DeviceControl: red:[%d], green:[%d], blue:[%d]\n", m_deviceState->red, m_deviceState->green, m_deviceState->blue);
    Serial.printf("DeviceControl: brightness:[%d]\n", m_deviceState->brightness);

    if ( (m_deviceState->effect != "") && (m_lightEffectsList.find(m_deviceState->effect) != m_lightEffectsList.end()) ) {
      FastLED.clear();
    } else {
      this->setColor(m_deviceState->red, m_deviceState->green, m_deviceState->blue, m_deviceState->brightness);
    }
  } else {
    Serial.println("DeviceControl: Switch the device off");
    m_deviceState->effect = "";
    FastLED.clear();
  }
  FastLED.show();
}


void DeviceControl::transition() {
  if (m_deviceState->state) { // Transition to ON
    m_current_brightness = m_current_brightness + (50.0/1000.0)*(float)m_deviceState->brightness/(float)m_deviceState->transition;
    if ( m_current_brightness >= m_deviceState->brightness) { // Stop transition
      m_deviceState->transition = 0;
      m_current_brightness = m_deviceState->brightness;
      m_inTransition = false;
    }
  } else { // Transition to OFF
    m_current_brightness = m_current_brightness - (50.0/1000.0)*(float)m_deviceState->brightness/(float)m_deviceState->transition;
     if ( m_current_brightness <= 0) { // Stop transition
      m_deviceState->transition = 0;
      m_current_brightness = 0;
      m_inTransition = false;
    }
  }
  this->setColor(m_deviceState->red, m_deviceState->green, m_deviceState->blue, m_current_brightness);
  FastLED.delay(50);
}


void DeviceControl::run()
{
  if ( m_inTransition ) {
    this->transition();
  } else {
    if (m_lightEffectsList.find(m_deviceState->effect) != m_lightEffectsList.end()) {
      m_lightEffectsList[ m_deviceState->effect ]();
    } else {
      m_deviceState->effect = "";
    }
  }

  FastLED.show();
}

/* --------------------------------------------------------------- */

/* ---- Effects ---- */
void DeviceControl::efLightUp()
{    
    if (m_effect_lightUp_index == (NUM_LEDS - 1)) {
      FastLED.clear();
    }

    leds[m_effect_lightUp_index] = CRGB(m_deviceState->red, m_deviceState->green, m_deviceState->blue );
    if (m_effect_lightUp_index == 0) {
      m_deviceState->effect = "";
      m_effect_lightUp_index = NUM_LEDS - 1;
      this->updateDeviceState();
    } else {
      m_effect_lightUp_index--;
      FastLED.delay(20);
    }
}

void DeviceControl::efLightDown()
{
  if (m_effect_lightDown_index == 0) {
    FastLED.clear();
  }
    
  leds[m_effect_lightDown_index] = CRGB(m_deviceState->red, m_deviceState->green, m_deviceState->blue );
  if (m_effect_lightDown_index == (NUM_LEDS - 1)) {
    m_deviceState->effect = "";
    m_effect_lightDown_index = 0;
    this->updateDeviceState();
  } else {
    m_effect_lightDown_index++;
    FastLED.delay(20);
  }
}


void DeviceControl::efDimUp()
{
    for (uint16_t i = 0; i < (NUM_LEDS - m_effect_lightUp_index); i++) {
      leds[NUM_LEDS - 1 - i].fadeToBlackBy(1);
    }

    EVERY_N_MILLISECONDS( 50 ) {
      if (m_effect_lightUp_index != 0) {
        m_effect_lightUp_index--;
      }
    }

    uint32_t pixel_0_color = (((uint32_t)leds[0].red << 16) | ((long)leds[0].green << 8 ) | (long)leds[0].blue);
    if ( pixel_0_color == 0 ) {
      FastLED.clear();
      m_deviceState->effect = "";
      m_deviceState->state = false;
      m_effect_lightUp_index = NUM_LEDS - 1;
      this->updateDeviceState();
    }
}

void DeviceControl::efDimDown()
{
  //TODO: To be implemented

  m_deviceState->effect = "";
  m_deviceState->state = false;
  m_effect_lightDown_index = 0;
  this->updateDeviceState();
}
