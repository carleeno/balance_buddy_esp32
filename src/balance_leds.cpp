#define FASTLED_INTERNAL
#include "FastLED.h"
#include "config.h"

class BalanceLEDs
{
private:
  bool directionIsForward;
  int targetBrightness;

  CRGB forward[NUM_LEDS_FORWARD];
  CRGB backward[NUM_LEDS_BACKWARD];

  // Helper function that blends one uint8_t toward another by a given amount
  void nblendU8TowardU8(uint8_t &cur, const uint8_t target, uint8_t amount)
  {
    if (cur == target)
      return;

    if (cur < target)
    {
      uint8_t delta = target - cur;
      delta = scale8_video(delta, amount);
      cur += delta;
    }
    else
    {
      uint8_t delta = cur - target;
      delta = scale8_video(delta, amount);
      cur -= delta;
    }
  }

  // Blend one CRGB color toward another CRGB color by a given amount.
  // Blending is linear, and done in the RGB color space.
  // This function modifies 'cur' in place.
  CRGB fadeTowardColor(CRGB &cur, const CRGB &target, uint8_t amount)
  {
    nblendU8TowardU8(cur.red, target.red, amount);
    nblendU8TowardU8(cur.green, target.green, amount);
    nblendU8TowardU8(cur.blue, target.blue, amount);
    return cur;
  }

  // Fade an entire array of CRGBs toward a given background color by a given amount
  // This function modifies the pixel array in place.
  void fadeTowardColor(CRGB *L, uint16_t N, const CRGB &bgColor, uint8_t fadeAmount)
  {
    for (uint16_t i = 0; i < N; i++)
    {
      fadeTowardColor(L[i], bgColor, fadeAmount);
    }
  }

public:
  void setup()
  {
    FastLED.addLeds<LED_TYPE, LED_PIN_FORWARD, COLOR_ORDER>(forward, NUM_LEDS_FORWARD).setCorrection(TypicalLEDStrip);
    FastLED.addLeds<LED_TYPE, LED_PIN_BACKWARD, COLOR_ORDER>(backward, NUM_LEDS_BACKWARD).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(0.0);

    if (STARTUP_FORWARD)
    {
      // Default to forward
      directionIsForward = true;
      fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_FORWARD, 75);
      fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_BACKWARD, 75);
    }
    else
    {
      directionIsForward = false;
      fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_BACKWARD, 75);
      fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_FORWARD, 75);
    }
    FastLED.show();
  }

  void loop(double erpm, uint16_t switchState)
  {
    // Latching behavior, if you know, you know.
    if (erpm > 300)
    {
      directionIsForward = true;
    }
    else if (erpm < -300)
    {
      directionIsForward = false;
    }

    if (directionIsForward)
    {
      fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_FORWARD, 75);
      fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_BACKWARD, 75);
    }
    else
    {
      fadeTowardColor(forward, NUM_LEDS_FORWARD, COLOR_BACKWARD, 75);
      fadeTowardColor(backward, NUM_LEDS_BACKWARD, COLOR_FORWARD, 75);
    }

    if (switchState == 0)
    {
      targetBrightness = BRIGHTNESS * SWITCH_OFF_BRIGHTNESS_SCALE;
    }
    else
    {
      targetBrightness = BRIGHTNESS;
    }

    // Fade Brightness
    if (FastLED.getBrightness() > targetBrightness)
    {
      FastLED.setBrightness(max(FastLED.getBrightness() - 10, targetBrightness));
    }
    else if (FastLED.getBrightness() < targetBrightness)
    {
      FastLED.setBrightness(min(FastLED.getBrightness() + 35, targetBrightness));
    }

    FastLED.show();
  }
};
