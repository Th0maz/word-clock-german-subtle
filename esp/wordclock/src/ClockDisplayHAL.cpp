#include "ClockDisplayHAL.h"

ClockDisplayHAL::WordMapping const ClockDisplayHAL::WORDS_TO_LEDS[] = {
        {"HOUR_0", 50, 52},
        {"HOUR_1", 49, 52},
        {"HOUR_2", 30, 33},
        {"HOUR_3", 37, 40},
        {"HOUR_4", 25, 28},
        {"HOUR_5", 42, 45},
        {"HOUR_6", 12, 16},
        {"HOUR_7", 66, 71},
        {"HOUR_8", 18, 21},
        {"HOUR_9", 54, 57},
        {"HOUR_10", 4, 7},
        {"HOUR_11", 9, 11},
        {"HOUR_12", 60, 64},
        {"OCLOCK", 0, 2},
        {"PAST", 92, 95},
        {"TO", 79, 81},
        {"MINUTES", 84, 90},
        {"THIRTY", 74, 77},
        {"TWENTY", 99, 105},
        {"TWENTYFIVE", 74, 77},
        {"FIVE", 108, 111},
        {"TEN", 121, 124},
        {"FIFTEEN", 113, 119},
        {"IS", 126, 128},
        {"IT", 130, 131}};

ClockDisplayHAL::ClockDisplayHAL(uint8_t pin, uint8_t brightness)
    : pixels(NUM_LEDS, pin, NEO_GRB + NEO_KHZ800), brightness(brightness)
{
}

void ClockDisplayHAL::setup()
{
    pixels.setBrightness(255);
    pixels.begin();
    pixels.show();
}

void ClockDisplayHAL::displayWord(const String &word, uint32_t color)
{
    for (auto mapping : WORDS_TO_LEDS)
    {
        if (word.equals(mapping.word))
        {
            for (uint8_t i = mapping.start; i <= mapping.end; ++i)
            {
                pixels.setPixelColor(i, color);
            }
            break;
        }
    }
}

uint16_t ClockDisplayHAL::cartesianToWordClockLEDStripIndex(uint8_t x, uint8_t y)
{
    uint16_t row_index;
    uint16_t index;

    if (y % 2 == 0)
    {
        row_index = NUM_LEDS - (y * WIDTH);
        index = row_index - (x + 1);
    }
    else
    {
        row_index = NUM_LEDS - ((y + 1) * WIDTH);
        index = row_index + x;
    }

    if (index < 0 || index >= NUM_LEDS)
    {
        return 0;
    }

    return index;
}

void ClockDisplayHAL::setPixel(uint8_t x, uint8_t y, uint32_t color)
{
    uint16_t index = cartesianToWordClockLEDStripIndex(x, y);
    pixels.setPixelColor(index, color);
}

void ClockDisplayHAL::clearPixels(bool show)
{
    pixels.clear();
    if (show)
    {
        pixels.show();
    }
}

void ClockDisplayHAL::show()
{
    pixels.show();
}
