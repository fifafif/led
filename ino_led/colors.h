
uint32_t getColor(float r, float g, float b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

uint32_t wheel(byte WheelPos, float brightness) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return getColor((255 - WheelPos * 3) * brightness, 0, WheelPos * 3 * brightness);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return getColor(0, (WheelPos * 3) * brightness, (255 - WheelPos * 3) * brightness);
  }
  WheelPos -= 170;
  return getColor(WheelPos * 3 * brightness, (255 - WheelPos * 3) * brightness, 0);
}

void rgbFromWheel(byte WheelPos, byte &redValue, byte &greenValue, byte &blueValue) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    redValue = 255 - WheelPos * 3;
    greenValue = 0;
    blueValue = WheelPos * 3;
    return;
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    redValue = 0;
    greenValue = WheelPos * 3;
    blueValue = 255 - WheelPos * 3;
    return;
  }
  WheelPos -= 170;
  redValue = WheelPos * 3;
  greenValue = 255 - WheelPos;
  blueValue = 0;
}