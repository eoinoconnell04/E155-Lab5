/*
Author: Eoin O'Connell
Email: eoconnell@hmc.edu
Date: Nov. 2, 2025
File function: Polling version of quadrature encoder velocity and direction measurement.
*/

#include "main.h"

#define A_PIN PA6 
#define B_PIN PA9

volatile uint32_t last_time = 0;
volatile uint32_t current_time = 0; 
volatile int direction = 0;   // +1 or -1
volatile float velocity = 0;  // ticks per second

// Function Prototypes
void initTimer(void);
void updateVelocity(void);

int main(void) {

    configureFlash();

    // Use 80 MHz PLL
    configureClock();
    
    // Enable GPIO ports
    gpioEnable(GPIO_PORT_A);

    // Configure encoder pins as inputs with pull-ups
    pinMode(A_PIN, GPIO_INPUT);
    pinMode(B_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (_VAL2FLD(GPIO_PUPDR_PUPD6, 0b01)); // PA6 pull-up
    GPIOA->PUPDR |= (_VAL2FLD(GPIO_PUPDR_PUPD9, 0b01)); // PA9 pull-up

    // Initialize delay timer for printing values
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;   
    initTIM(DELAY_TIM);

    // Initialize 32-bit timer for measuring time between pulses
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initCounterTIM(COUNT_TIM);

    // Variables to track previous encoder state
    int prev_a = digitalRead(A_PIN);
    int prev_b = digitalRead(B_PIN);

    while (1) {
        delay_millis(DELAY_TIM, 1);  // small delay for stability

        int cur_a = digitalRead(A_PIN);
        int cur_b = digitalRead(B_PIN);

        // Check if any edge occurred
        if ((cur_a != prev_a) || (cur_b != prev_b)) {
            // Update time and velocity
            last_time = current_time;
            current_time = TIM2->CNT;

            int delta = current_time - last_time;

            if (delta > 0) {
                velocity = 1000000.0f / (float)delta / 408.0f / 4.0f; // timer at 1 MHz, divide by PPR and edges
            }

            // Determine direction based on quadrature logic
            if (prev_a == prev_b) {
                direction = (cur_a == cur_b) ? -1 : +1;
            } else {
                direction = (cur_a == cur_b) ? +1 : -1;
            }

            prev_a = cur_a;
            prev_b = cur_b;
        }

        // Check for long stop (no pulses)
        volatile uint32_t now = TIM2->CNT;
        if ((now - current_time) > 100000) {
            velocity = 0;
        }

        // Print current velocity and direction
        if (direction == 1) {
            printf("%.2f Hz CW\n", velocity);
        } else {
            printf("%.2f Hz CCW\n", velocity);
        }

        delay_millis(DELAY_TIM, 200); // periodic print
    }
}
