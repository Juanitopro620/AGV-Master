/*
 * line_sensors.c
 *
 * Created on: May 31, 2026
 * Author: JP
 */

#include "line_sensors.h"

// Global sticky note updated by interrupts. We start assuming it's centered.
volatile int actual_reading = STRAIGHT_LINE;

//Function development

void Sensors_Init(void) {
	//1. Clock
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  // PortA enable
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;  // Router enable

	//2. Pin cleaning = inputs (PA0, PA1, PA2)
	GPIOA->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2);

	//3. Router (EXTI connected to 0, 1 and 2 to port A)
	SYSCFG->EXTICR[0] &= ~( (0xF<<0) | (0xF<<4) | (0xF<<8) );

	//4. Trigger (Detect both Rising and Falling edges)
	EXTI->RTSR |= ((1<<0) | (1<<1) | (1<<2));  // Enable Rising Edge (0 to 1)
	EXTI->FTSR |= ((1<<0) | (1<<1) | (1<<2));  // Enable Falling Edge (1 to 0)

	// 5. Mask and Core Interrupt Controller (Open the gates)
	EXTI->IMR |= ((1<<0) | (1<<1) | (1<<2));  // Unmask EXTI lines 0, 1, and 2
	NVIC_EnableIRQ(EXTI0_1_IRQn);             // Enable NVIC channel for PA0 and PA1
	NVIC_EnableIRQ(EXTI2_3_IRQn);             // Enable NVIC channel for PA2
}

int Sensor_Lecture(void) {
    // Read and mask the exact pins (PA0, PA1, PA2)
    return (GPIOA->IDR & (GPIO_IDR_0 | GPIO_IDR_1 | GPIO_IDR_2));
}

// ISR for Pins 0 and 1
void EXTI0_1_IRQHandler(void) {
    // Check and clear pending flag for Pin 0
    if (EXTI->PR & (1 << 0)) {
        EXTI->PR = (1 << 0);
    }
    // Check and clear pending flag for Pin 1
    if (EXTI->PR & (1 << 1)) {
        EXTI->PR = (1 << 1);
    }

    // Update the global sticky note with the real world state
    actual_reading = Sensor_Lecture();
}

// ISR for Pins 2 and 3
void EXTI2_3_IRQHandler(void) {
    // Check and clear pending flag for Pin 2
    if (EXTI->PR & (1 << 2)) {
        EXTI->PR = (1 << 2);
    }

    // Update the global sticky note with the real world state
    actual_reading = Sensor_Lecture();
}
