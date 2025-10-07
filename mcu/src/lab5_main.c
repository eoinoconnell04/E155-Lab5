/*
Author: Eoin O'Connell
Email: eoconnell@hmc.edu
Date: Oct. 6, 2025
File function: This program uses an algorithm to sense quadrature encoder pulses and convert these into motor velocity and direction.
*/

#include "main.h"

#define A_PIN  0 // trying to use PA0 & PA1, might be pin 6 7
#define B_PIN  1

volatile uint32_t last_time = 0;
volatile int direction = 0;   // +1 or -1
volatile float velocity = 0;  // ticks per second

// Function Prototypes
void initTimer(void);
void configureInterrupts(void);
void updateVelocity(void);

// Main Function
int main(void) {
    // Use 80 Mhz PLL
    configureClock();
    
    // Enable GPIO ports
    gpioEnable(GPIO_PORT_A);

    // Configure encoder pins as inputs with pull-ups
    pinMode(A_PIN, GPIO_INPUT);
    pinMode(B_PIN, GPIO_INPUT);
    GPIOA->PUPDR |= (_VAL2FLD(GPIO_PUPDR_PUPD0, 0b01)); // PA0 pull-up
    GPIOA->PUPDR |= (_VAL2FLD(GPIO_PUPDR_PUPD1, 0b01)); // PA1 pull-up

    // Initialize delay timer for printing values
    RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;   
    initTIM(DELAY_TIM);

    // Initialize 32 bit timer for measuring time between pulses
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
    initCounterTIM(COUNT_TIM);

    configureInterrupts();

    // enable interrupts globally
    __enable_irq();

    while (1) {
        delay_millis(DELAY_TIM, 200);
        if (direction == 1){
            printf("%.2f Hz CW\n", velocity);
        }
        else {
            printf("%.2f Hz CCW\n", velocity);
        }
    }
}

void configureInterrupts(void) {
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Configure EXTI lines for PA0 and PA1
    SYSCFG->EXTICR[0] &= ~((0xF << 0) | (0xF << 4));  // EXTI0 & EXTI1 from PA

    // Unmask interrupts
    EXTI->IMR1 |= (1 << 0) | (1 << 1);

    // Trigger on both rising and falling edges for both pins
    EXTI->RTSR1 |= (1 << 0) | (1 << 1); 
    EXTI->FTSR1 |= (1 << 0) | (1 << 1);

    // Enable EXTI lines in NVIC
    NVIC->ISER[0] |= (1 << EXTI0_IRQn);
    NVIC->ISER[0] |= (1 << EXTI1_IRQn);
}

// Reads timer to update velocity
void updateVelocity(void) {
    uint32_t current_time = TIM2->CNT; // read current time
    TIM2->CNT = 0; // reset timer for next interval

    // Compute velocity (ticks per second)
    if (current_time != 0)
        velocity = 1000000 / current_time / 408 / 4; //  timer is at 1 MHz, divide by # of ticks, PPR, and # of edges 

}

// Interrupt handlers

// Interrupt signalling that pin A changed
void EXTI0_IRQHandler(void) {
    if (EXTI->PR1 & (1 << 0)) {
        EXTI->PR1 |= (1 << 0); // clear pending
        
        int a = readPin(A_PIN);
        int b = readPin(B_PIN);

        if (a == b)
            direction = -1;  // reverse
        else
            direction = +1;  // forward

        updateVelocity();
    }
}

// Interrupt signalling that pin B changed
void EXTI1_IRQHandler(void) {
    if (EXTI->PR1 & (1 << 1)) {
        EXTI->PR1 |= (1 << 1); // clear pending

        int a = readPin(A_PIN);
        int b = readPin(B_PIN);

        if (a == b)
            direction = +1;  // forward
        else
            direction = -1;  // reverse

        updateVelocity();
    }
}
