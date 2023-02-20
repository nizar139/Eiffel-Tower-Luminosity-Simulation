#include "mbed.h"
#include "PololuLedStrip.h"

// Define the two LED strips , marche , en vrai pas besoin de ces matrices
PololuLedStrip ledStrip1(D8);
PololuLedStrip ledStrip2(D9);


// Define the number of LEDs for each strip
#define LED_COUNT_1 304
#define LED_COUNT_2 304


// Create arrays for the colors of each LED for each strip
rgb_color colors1[LED_COUNT_1];
rgb_color colors2[LED_COUNT_2];


Timer timer;

int main()
{
    timer.start();
    
    // Initialize variables
    int ind = 0;
    int program = 0;
    int period = 50;
    uint32_t time, tp;
    int i;
    
    time = timer.read_ms();
    while (1)
    {
        // Wait for the period
        while ((tp = timer.read_ms()) < (time + period)) {}
        time = tp;
        
        // Change programs regularly
        program = (tp / 50000) % 2;
        
        // Update the colors arrays based on the program
        if (program == 0)
        {
            // Program 1: Gradual color change
            ind = (ind + 1) % 768;
            for (i = 0; i < LED_COUNT_1; i++)
            {
                if (ind < 256)
                {
                    colors1[i] = (rgb_color){ind, 255 - ind, 0};
                }
                else if (ind < 512)
                {
                    colors1[i] = (rgb_color){0, ind - 256, 255 - ind + 256};
                }
                else
                {
                    colors1[i] = (rgb_color){ind - 512, 0, 255 - ind + 512};
                }
            }
            
        }
        else if (program == 1)
        {
            // Program 2: Flickering effect
            for (i = 0; i < LED_COUNT_2; i++)
            {
                colors2[i] = (rgb_color){rand() % 256, rand() % 256, rand() % 256};
            }
        }
            ledStrip1.write(colors1, LED_COUNT_1);
            ledStrip2.write(colors2, LED_COUNT_2);
    }
}