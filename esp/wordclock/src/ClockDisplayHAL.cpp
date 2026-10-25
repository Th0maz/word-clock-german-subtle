#include "ClockDisplayHAL.h"

ClockDisplayHAL::WordMapping const ClockDisplayHAL::WORDS_TO_LEDS[] = {
    {"HOUR_1", 20, 22},
    {"HOUR_2", 45, 47},
    {"HOUR_3", 15, 19},
    {"HOUR_4", 67, 70},
    {"HOUR_5", 40, 43},
    {"HOUR_6", 12, 14},
    {"HOUR_7", 55, 59},
    {"HOUR_8", 31, 35},
    {"HOUR_9", 36, 39},
    {"HOUR_10", 9, 11},
    {"HOUR_11", 24, 29},
    {"HOUR_12", 48, 53},
    {"OCLOCK", 0, 5},
    {"PAST", 60, 63},
    {"TO", 63, 64},
    {"MINUTES", 77, 83},
    {"THIRTY", 84, 89},
    {"TWENTY", 102, 107},
    {"TWENTYFIVE", 98, 107},
    {"FIVE", 98, 101},
    {"TEN", 91, 93},
    {"FIFTEEN", 110, 116},
    {"IS", 127, 128},
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
    memset(shownColors, 0, sizeof(shownColors));
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
        this->show();
    }
}

void ClockDisplayHAL::show()
{
    pixels.show();

    // Merken, was gerade tatsächlich leuchtet - Startpunkt für das nächste Überblenden
    for (uint16_t i = 0; i < NUM_LEDS; ++i)
    {
        shownColors[i] = pixels.getPixelColor(i);
    }
}

// Linear zwischen zwei Farbkanälen mischen, t = 0..256
static inline uint8_t lerp8(uint8_t a, uint8_t b, uint16_t t)
{
    return (uint8_t)(((uint16_t)a * (256 - t) + (uint16_t)b * t) >> 8);
}

static uint32_t blendColor(uint32_t from, uint32_t to, uint16_t t)
{
    uint8_t r = lerp8((from >> 16) & 0xFF, (to >> 16) & 0xFF, t);
    uint8_t g = lerp8((from >> 8) & 0xFF, (to >> 8) & 0xFF, t);
    uint8_t b = lerp8(from & 0xFF, to & 0xFF, t);
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Zeigt den aktuell im Puffer aufgebauten Zustand, indem vom zuletzt
// angezeigten Zustand weich übergeblendet wird. Pixel, die neu leuchten,
// werden eingeblendet, Pixel, die ausgehen, ausgeblendet, unveränderte
// Pixel bleiben konstant.
void ClockDisplayHAL::showFaded(uint16_t durationMs)
{
    uint32_t target[NUM_LEDS];
    bool changed = false;

    for (uint16_t i = 0; i < NUM_LEDS; ++i)
    {
        target[i] = pixels.getPixelColor(i);
        if (target[i] != shownColors[i])
        {
            changed = true;
        }
    }

    if (!changed)
    {
        // Nichts zu überblenden - Anzeige nur wiederherstellen
        for (uint16_t i = 0; i < NUM_LEDS; ++i)
        {
            pixels.setPixelColor(i, target[i]);
        }
        pixels.show();
        return;
    }

    uint32_t startMs = millis();
    uint32_t elapsed;
    while ((elapsed = millis() - startMs) < durationMs)
    {
        uint16_t t = (uint16_t)((elapsed * 256UL) / durationMs);
        for (uint16_t i = 0; i < NUM_LEDS; ++i)
        {
            pixels.setPixelColor(i, blendColor(shownColors[i], target[i], t));
        }
        pixels.show();
        yield(); // WLAN-Stack / Watchdog bedienen
    }

    // Endzustand exakt setzen
    for (uint16_t i = 0; i < NUM_LEDS; ++i)
    {
        pixels.setPixelColor(i, target[i]);
        shownColors[i] = target[i];
    }
    pixels.show();
}
