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
  uint8_t realRed = map(red, 0, 255, 0, brightness);
  uint8_t realGreen = map(green, 0, 255, 0, brightness);
  uint8_t realBlue = map(blue, 0, 255, 0, brightness);
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
      //FastLED.clear();
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

    uint8_t realRed = map(m_deviceState->red, 0, 255, 0, m_deviceState->brightness);
    uint8_t realGreen = map(m_deviceState->green, 0, 255, 0, m_deviceState->brightness);
    uint8_t realBlue = map(m_deviceState->blue, 0, 255, 0, m_deviceState->brightness);

    leds[m_effect_lightUp_index] = CRGB(realRed, realGreen, realBlue );
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

  uint8_t realRed = map(m_deviceState->red, 0, 255, 0, m_deviceState->brightness);
  uint8_t realGreen = map(m_deviceState->green, 0, 255, 0, m_deviceState->brightness);
  uint8_t realBlue = map(m_deviceState->blue, 0, 255, 0, m_deviceState->brightness);

  leds[m_effect_lightDown_index] = CRGB(realRed, realGreen, realBlue );
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
    leds[NUM_LEDS - 1 - i].fadeToBlackBy(10);
  }

  if (m_effect_lightUp_index != 0) {
    m_effect_lightUp_index--;
  }

  uint32_t pixel_0_color = (((uint32_t)leds[0].red << 16) | ((long)leds[0].green << 8 ) | (long)leds[0].blue);
  if ( pixel_0_color == 0 ) {
    FastLED.clear();
    m_deviceState->effect = "";
    m_deviceState->state = false;
    m_effect_lightUp_index = NUM_LEDS - 1;
    this->updateDeviceState();
    if (m_sendMessageState) {
      m_sendMessageState();
    }
  }

  FastLED.delay(40);
}

void DeviceControl::efDimDown()
{
  for (uint16_t i = 0; i < m_effect_lightDown_index; i++) {
    leds[i].fadeToBlackBy(10);
  }

  if (m_effect_lightDown_index != NUM_LEDS) {
    m_effect_lightDown_index++;
  }

  uint32_t pixel_N_color = (((uint32_t)leds[NUM_LEDS-1].red << 16) | ((long)leds[NUM_LEDS-1].green << 8 ) | (long)leds[NUM_LEDS-1].blue);
  if ( pixel_N_color == 0 ) {
    FastLED.clear();
    m_deviceState->effect = "";
    m_deviceState->state = false;
    m_effect_lightDown_index = 0;
    this->updateDeviceState();
    if (m_sendMessageState) {
      m_sendMessageState();
    }
  }

  FastLED.delay(40);
}

void DeviceControl::efStartLight()
{
  uint8_t realRed = map(m_deviceState->red, 0, 255, 0, m_deviceState->brightness);
  uint8_t realGreen = map(m_deviceState->green, 0, 255, 0, m_deviceState->brightness);
  uint8_t realBlue = map(m_deviceState->blue, 0, 255, 0, m_deviceState->brightness);

  for (uint8_t i = 0; i < 3; i++) {
    if ( i < 4 ) {
      leds[i] = CRGB(realRed, realGreen, realBlue);
    } else {
      leds[i] = CRGB(0, 0, 0);
    }
  }

  FastLED.delay(50);
}


void DeviceControl::efEndLight()
{
  uint8_t realRed = map(m_deviceState->red, 0, 255, 0, m_deviceState->brightness);
  uint8_t realGreen = map(m_deviceState->green, 0, 255, 0, m_deviceState->brightness);
  uint8_t realBlue = map(m_deviceState->blue, 0, 255, 0, m_deviceState->brightness);

  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    if ( i < 4 ) {
      leds[NUM_LEDS - 1 - i] = CRGB(realRed, realGreen, realBlue);
    } else {
      leds[NUM_LEDS - 1 - i] = CRGB(0, 0, 0);
    }
  }

  FastLED.delay(50);
}


void DeviceControl::efChaos()
{
  for (int i = 0; i < NUM_LEDS; i++) {
    if ( (chaosEffectHelper[i].up == false) && (chaosEffectHelper[i].brightness == 0) ) {
      chaosEffectHelper[i].hue = random(0, 255);
      chaosEffectHelper[i].saturation = 255;
      chaosEffectHelper[i].brightness = 0;
      chaosEffectHelper[i].maxBrightness = random(100, 255);
      chaosEffectHelper[i].up = true;
    } else if ((chaosEffectHelper[i].up == true) && (chaosEffectHelper[i].brightness >= 0)) {
      chaosEffectHelper[i].brightness = chaosEffectHelper[i].brightness + 1;
      chaosEffectHelper[i].up = chaosEffectHelper[i].brightness < chaosEffectHelper[i].maxBrightness;
    } else if ((chaosEffectHelper[i].up == false) && (chaosEffectHelper[i].brightness > 0)) {
      chaosEffectHelper[i].brightness = chaosEffectHelper[i].brightness - 1;
    }
  }
  for (byte j=0; j<NUM_LEDS; j++) {
    leds[j] = CHSV(chaosEffectHelper[j].hue, chaosEffectHelper[j].saturation, chaosEffectHelper[j].brightness );
  }
  FastLED.delay(20);
}
