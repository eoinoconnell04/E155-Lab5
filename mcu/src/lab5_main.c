/*
Author: Eoin O'Connell
Email: eoconnell@hmc.edu
Date: Oct. 6, 2025
File function: This program uses an algorithm to sense quadrature encoder pulses and convert these into motor velocity and direction.
*/

#include "main.h"

#define A_PIN PA6 // trying to use PA0 & PA1, might be pin 6 7
#define B_PIN PA9

volatile uint32_t last_time = 0;
volatile int direction = 0;   // +1 or -1
volatile float velocity = 0;  // ticks per second
static volatile uint8_t last_a; 
static volatile uint8_t last_b; 

// Function Prototypes
void initTimer(void);
void configureInterrupts(void);
void updateVelocity(void);

// Main Function
int main(void) {

    configureFlash();

    // Use 80 Mhz PLL
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

    // Configure EXTI lines for PA6 and PA9
    SYSCFG->EXTICR[1] |= _VAL2FLD(SYSCFG_EXTICR2_EXTI6, 0b000); // Select PA6
    SYSCFG->EXTICR[2] |= _VAL2FLD(SYSCFG_EXTICR3_EXTI9, 0b000); // Select PA9

    // Unmask interrupts
    EXTI->IMR1 |= (1 << 6) | (1 << 9);

    // Trigger on both rising and falling edges for both pins
    EXTI->RTSR1 |= (1 << 6) | (1 << 9); 
    EXTI->FTSR1 |= (1 << 6) | (1 << 9);

    // Enable EXTI lines in NVIC
    NVIC->ISER[0] |= (1 << EXTI9_5_IRQn);
}

// Reads timer to update velocity
void updateVelocity(void) {
    uint32_t current_time = TIM2->CNT; // read current time
    TIM2->CNT = 0; // reset timer for next interval

    // Compute velocity (ticks per second)
    if (current_time != 0 && current_time < 5000) {
        velocity = 1000000 / current_time / 408 / 4; //  timer is at 1 MHz, divide by # of ticks, PPR, and # of edges 
    } else {
        velocity = 0; // at super slow speeds set velocity to 0
    }

}

// Interrupt handlers

// Interrupt signalling that pin either pin changed
void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR1 & (1 << 6)) {
        EXTI->PR1 |= (1 << 6); // clear pending
        
        updateVelocity();

        int a = digitalRead(A_PIN);
        int b = digitalRead(B_PIN);

        if (a == b)
            direction = -1;  // reverse
        else
            direction = +1;  // forward
    }

    if (EXTI->PR1 & (1 << 9)) {
        EXTI->PR1 |= (1 << 9); // clear pending
        
        updateVelocity();

        int a = digitalRead(A_PIN);
        int b = digitalRead(B_PIN);

        if (a == b)
            direction = +1;  // forward
        else
            direction = -1;  // reverse
    }
}
