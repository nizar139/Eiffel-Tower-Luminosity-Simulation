#include "mbed.h"
#include "PololuLedStrip.h"

// Define the two LED strips
PololuLedStrip ledStrip1(D8);
PololuLedStrip ledStrip2(D9);

// Define the number of LEDs for each strip
#define LED_COUNT_1 304
#define LED_COUNT_2 304

// Create arrays for the colors of each LED for each strip
rgb_color colors1[LED_COUNT_1];
rgb_color colors2[LED_COUNT_2];

// Timer for the breathing effect
Timer timer;

int main()
{
    timer.start();
    
    // Initialize variables
    int program = 0;
    int period = 50;
    int i;
    float t;
    float brightness1, brightness2;
    uint32_t time, tp;
    
    // Set the initial color for both strips
    for (i = 0; i < LED_COUNT_1; i++)
    {
        colors1[i] = (rgb_color){255, 0, 0};
    }
    for (i = 0; i < LED_COUNT_2; i++)
    {
        colors2[i] = (rgb_color){0, 0, 255};
    }
    
    time = timer.read_ms();
    while (1)
    {
        // Wait for the period
        while ((tp = timer.read_ms()) < (time + period)) {}
        time = tp;
        
        // Get the current time in seconds
        t = timer.read();
        
        // Update the brightness based on a sine wave
        brightness1 = (sin(t * 2 * 3.14159 * 0.5) + 1) / 2;
        brightness2 = (sin(t * 2 * 3.14159 * 0.5 + 3.14159) + 1) / 2;
        
        // Update the colors array based on the brightness
        for (i = 0; i < LED_COUNT_1; i++)
        {
            colors1[i].red = (uint8_t)(colors1[i].red * brightness1);
            colors1[i].green = (uint8_t)(colors1[i].green * brightness1);
            colors1[i].blue = (uint8_t)(colors1[i].blue * brightness1);
        }
        for (i = 0; i < LED_COUNT_2; i++)
        {
            colors2[i].red = (uint8_t)(colors2[i].red * brightness2);
            colors2[i].green = (uint8_t)(colors2[i].green * brightness2);
            colors2[i].blue = (uint8_t)(colors2[i].blue * brightness2);
        }
        
        // Write the new colors to the LED strips
        ledStrip1.write(colors1, LED_COUNT_1);
        ledStrip2.write(colors2, LED_COUNT_2);
    }
}
