/*
 * motors.h
 *
 *  Created on: May 31, 2026
 *      Author: JP
 */

#ifndef MOTORS_H_
#define MOTORS_H_

#include "stm32f051x8.h"

//Function declaration
void Motors_Init(void);
void Set_Motor_Speed(int left_speed, int right_speed);

#endif /* MOTORS_H_ */
