/* HAL-only entry function */
#include "hal_data.h"

#include <stdio.h>
#include <string.h>

// Buffer Size
#define OUTPUT_BUFFER_SIZE 1024

// Tick Rate
#define COUNTS_PER_MILLISECOND  (120E6 / 1000)

#define POT_CHANNEL 0
#define TEMP_CHANNEL 28
#define INTERNAL_VREF_CHANNEL 29

#define USE_VT100   (true)


// Buffers
char outputBuffer[OUTPUT_BUFFER_SIZE];

// Flags
volatile bool transmitComplete;

int _write(int file, char *buffer, int count);
int _write(int file, char *buffer, int count)
{
    // As far as I know, there isn't a way to retrieve how many
    // bytes were send on using the uart->write function if it does not return
    // SSP_SUCCESS (unless we want to use the tx interrupt function and a global counter
    // so, we will send each character one by one instead.
    int bytesTransmitted = 0;

    for (int i = 0; i < count; i++)
    {
        // Start Transmission
        transmitComplete = false;
        g_uart.p_api->write (g_uart.p_ctrl, (uint8_t const *) (buffer + i), 1);
        while (!transmitComplete)
        {
        }

        bytesTransmitted++;
    }

    return bytesTransmitted;
}

// Callback Function for UART interrupts
void user_uart_callback(uart_callback_args_t * p_args)
{
    // Get Event Type
    switch (p_args->event)
    {
        // Transmission Complete
        case UART_EVENT_TX_COMPLETE:
            transmitComplete = true;
        break;
        default:
        break;
    }
}

void hal_entry(void)
{
    // Variable to hold ADC Data
    uint16_t adcCounts;
    uint16_t temperatureCounts;
    uint16_t internalVRefCounts;
    float adcVoltage;
    float temperatureVoltage;
    float temperatureDegreesC;
    float temperatureDegreesF;
    float internalVRefVoltage;

    // Variable to hold counts
    timer_size_t counts = 0;

    // Open UART
    g_uart.p_api->open (g_uart.p_ctrl, g_uart.p_cfg);

    // Disable Output Buffering
    setvbuf ( stdout, NULL, _IONBF, OUTPUT_BUFFER_SIZE);

    // Open the timer using the configured options from the configurator
    g_timer.p_api->open (g_timer.p_ctrl, g_timer.p_cfg);

    // Open the ADC
    g_adc.p_api->open (g_adc.p_ctrl, g_adc.p_cfg);

    // Configure Scan
    g_adc.p_api->scanCfg (g_adc.p_ctrl, g_adc.p_channel_cfg);

    // Use TTY100 commands to clear screen and reset screen pointer
    if (USE_VT100 == true)
    {
        printf ("\033[2J"); // Clear Screen
        printf ("\033[H"); // Return Home
        printf ("\033[3J"); // Clear Back Buffer
    }

    // Print Header
    printf ("Lesson 008: ADC\r\n");
    printf ("Potentiometer Counts: 0\r\n");
    printf ("Potentiometer  Value: 0\r\n");
    printf ("  Temperature Counts: 0\r\n");
    printf ("   Temperature Value: 0\r\n");
    printf ("       Temperature C: 0\r\n");
    printf ("       Temperature F: 0\r\n");
    printf ("Internal VRef Counts: 0\r\n");
    printf ("Internal VRef  Value: 0\r\n");

    while (true)
    {
        // Start ADC Scan
        g_adc.p_api->scanStart (g_adc.p_ctrl);

        // Wait for scan to complete
        while (true)
        {
            if (g_adc.p_api->scanStatusGet (g_adc.p_ctrl) == SSP_SUCCESS)
                break;
        }

        // Read ADC
        g_adc.p_api->read (g_adc.p_ctrl, POT_CHANNEL, &adcCounts);
        g_adc.p_api->read (g_adc.p_ctrl, TEMP_CHANNEL, &temperatureCounts);
        g_adc.p_api->read (g_adc.p_ctrl, INTERNAL_VREF_CHANNEL, &internalVRefCounts);

        // Convert Counts to Voltage
        adcVoltage = ((adcCounts * 3.3f) / 4095.0f);
        temperatureVoltage = ((temperatureCounts * 3.3f) / 4095.0f);
        internalVRefVoltage = ((internalVRefCounts * 3.3f) / 4095.0f);

        // Convert Voltage to Degrees C and Degrees F
        temperatureDegreesC = (float)(((temperatureVoltage - 1.24f) / 0.0041f) - 25.0f);
        temperatureDegreesF = (float)(((temperatureDegreesC * 9.0f) / 5.0f) + 32.0f);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[8A"); // Move up 8 lines
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Counts
        printf ("%d\r\n", adcCounts);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Voltage
        printf ("%f\r\n", adcVoltage);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Voltage
        printf ("%d\r\n", temperatureCounts);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Voltage
        printf ("%f\r\n", temperatureVoltage);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Temp C
        printf ("%f\r\n", temperatureDegreesC);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Temp F
        printf ("%f\r\n", temperatureDegreesF);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Voltage
        printf ("%d\r\n", internalVRefCounts);

        // Prep terminal to updated values (if using VT100)
        if (USE_VT100 == true)
        {
            printf ("\033[22C"); // Move right 22 characters
            printf ("\033[K"); // Clear to end of line
        }

        // Print Voltage
        printf ("%f\r\n", internalVRefVoltage);

        // Wait 100ms before we do another read
        // Reset the timer to 0
        g_timer.p_api->reset (g_timer.p_ctrl);
        while (1)
        {
            // Get current counts
            g_timer.p_api->counterGet (g_timer.p_ctrl, &counts);

            // Check if 1000ms has elapsed => This should be a helper function at some point
            // Need to look if the PBCLK settings are stored in a define somewhere...
            if (counts > (100 * COUNTS_PER_MILLISECOND))
            {
                // Reset the timer to 0
                g_timer.p_api->reset (g_timer.p_ctrl);
                break;
            }
        }
    }
}
