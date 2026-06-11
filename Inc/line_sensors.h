/*
 * line_sensors.h
 *
 *  Created on: May 31, 2026
 *      Author: JP
 */

#ifndef LINE_SENSORS_H_
#define LINE_SENSORS_H_

#include "stm32f051x8.h"


#define RIGHT_LINE     (GPIO_IDR_0)  // PA0 - RIGHT
#define STRAIGHT_LINE  (GPIO_IDR_1)  // PA1 - CENTER
#define LEFT_LINE      (GPIO_IDR_2)  // PA2 - LEFT
#define CROSS_LINE     (GPIO_IDR_0 | GPIO_IDR_1 | GPIO_IDR_2) // All
#define LOST_LINE      (0x00)        // NO LINE

// Global variable updated automatically by EXTI interrupts
extern volatile int actual_reading;

// Function prototypes
void Sensors_Init(void);
int Sensor_Lecture(void);

#endif /* LINE_SENSORS_H_ */
