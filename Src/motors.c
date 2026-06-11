/*
 * motors.c
 *
 * Created on: May 31, 2026
 * Author: JP
 */

#include "motors.h"

void Motors_Init(void) {
    // 1. Clocks
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;  // Timer 3 (Motor Derecho)
    RCC->APB2ENR |= (RCC_APB2ENR_TIM16EN | RCC_APB2ENR_TIM17EN); // Timer 16 y 17 (Motor Izquierdo)

    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;  // Clock portB
    RCC->AHBENR |= RCC_AHBENR_GPIODEN;  // Clock portD

    /* DRIVER 1        DRIVER 2
     *
     * RPWM1 = PB9     RPWM2 = PB5    PWM FORWARD
     * LPWM1 = PB8     LPWM2 = PB4    PWM REVERSE
     * REN1 = PB7      REN2 = PB3     ENABLE FORWARD
     * LEN1 = PB6      LEN2 = PD2     ENABLE REVERSE
     */

    // ---------------------------- PWM PINS ----------------------------

    // Alternative PINS for PWM
    GPIOB->MODER &= ~(GPIO_MODER_MODER9 | GPIO_MODER_MODER8 | GPIO_MODER_MODER5 | GPIO_MODER_MODER4);
    GPIOB->MODER |= (GPIO_MODER_MODER9_1 | GPIO_MODER_MODER8_1 | GPIO_MODER_MODER5_1 | GPIO_MODER_MODER4_1);

    // Connect timer with pins, with AF (Alternative Function)
    GPIOB->AFR[0] &= ~((0xF << 16) | (0xF << 20)); // Clean Pins 4 and 5
    GPIOB->AFR[1] &= ~((0xF << 0)  | (0xF << 4));  // Clean Pins 8 and 9

    // Enrutamiento correcto de los pines
    GPIOB->AFR[0] |= ((1 << 16) | (1 << 20));      // AF1 para PB4 y PB5 (TIM3)
    GPIOB->AFR[1] |= ((2 << 0)  | (2 << 4));       // AF2 para PB8 y PB9 (TIM16 y TIM17)

    // ------------------------- TIMERS SETUP -------------------------

    // TIMER 3 (PB4 y PB5)
    TIM3->PSC = 2; // Frecuencia bajada a 20kHz
    TIM3->ARR = 799;
    TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC2M);
    TIM3->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2); // PWM mode Channel 1
    TIM3->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2); // PWM mode Channel 2
    TIM3->CCER |= (TIM_CCER_CC1E | TIM_CCER_CC2E);
    TIM3->CR1 |= TIM_CR1_CEN;

    // TIMER 16 y 17 (PB8 y PB9)
    TIM16->PSC = 2; // Frecuencia bajada a 20kHz
    TIM16->ARR = 799;
    TIM17->PSC = 2; // Frecuencia bajada a 20kHz
    TIM17->ARR = 799;

    TIM16->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM16->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2); // PWM mode 1 para TIM16_CH1

    TIM17->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM17->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2); // PWM mode 1 para TIM17_CH1

    TIM16->CCER |= TIM_CCER_CC1E;
    TIM17->CCER |= TIM_CCER_CC1E;

    // SEGURO PRINCIPAL: TIM16 y TIM17 necesitan activar el "Main Output Enable" para sacar PWM
    TIM16->BDTR |= TIM_BDTR_MOE;
    TIM17->BDTR |= TIM_BDTR_MOE;

    TIM16->CR1 |= TIM_CR1_CEN;
    TIM17->CR1 |= TIM_CR1_CEN;

    // ------------------------ ENABLE PINS ------------------------

    //Configure pins as Digital outputs (RENs and LENs)
    GPIOB->MODER &= ~(GPIO_MODER_MODER3 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
    GPIOB->MODER |= (GPIO_MODER_MODER3_0 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);

    GPIOD->MODER &= ~(GPIO_MODER_MODER2);
    GPIOD->MODER |= GPIO_MODER_MODER2_0;
}

void Set_Motor_Speed(int left_speed, int right_speed) {

    int pwm_left = left_speed;
    int pwm_right = right_speed;

    if (pwm_left > 799)  pwm_left = 799;
    if (pwm_left < -799) pwm_left = -799;
    if (pwm_right > 799) pwm_right = 799;
    if (pwm_right < -799) pwm_right = -799;

    // ---------- LEFT MOTOR LOGIC (TIM16 = PB8, TIM17 = PB9) ----------

    // PB7 (REN1) and PB6 (LEN1)
    if (left_speed > 0) {
        //FORWARD
        GPIOB->BSRR = (1 << 7) | (1 << 6);  // PB7 y PB6 ON
        TIM16->CCR1 = 0;                    // Reverse OFF (LPWM1 = PB8)
        TIM17->CCR1 = pwm_left;             // Forward ON (RPWM1 = PB9)
    }
    else if (left_speed < 0) {
        //REVERSE
        pwm_left = pwm_left * -1; // Convert to positive to CCR

        GPIOB->BSRR = (1 << 7) | (1 << 6);
        TIM17->CCR1 = 0;                    // Forward OFF (RPWM1 = PB9)
        TIM16->CCR1 = pwm_left;             // Reverse ON (LPWM1 = PB8)
    }
    else {
        // BRAKE
        GPIOB->BRR = (1 << 7) | (1 << 6);
        TIM17->CCR1 = 0;
        TIM16->CCR1 = 0;
    }

    // ---------- RIGHT MOTOR LOGIC (TIM3 = PB4, PB5) ----------

    // PB3 (REN2) and PD2 (LEN2)
    if (right_speed > 0) {
        // FORWARD
        GPIOB->BSRR = (1 << 3);             // PB3 ON
        GPIOD->BSRR = (1 << 2);             // PD2 ON
        TIM3->CCR1 = 0;                     // Reverse OFF (LPWM2 = PB4)
        TIM3->CCR2 = pwm_right;             // Forward ON (RPWM2 = PB5)
    }
    else if (right_speed < 0) {
        // REVERSE
        pwm_right = pwm_right * -1; // Convert to positive for CCR

        GPIOB->BSRR = (1 << 3);
        GPIOD->BSRR = (1 << 2);
        TIM3->CCR2 = 0;                     // Forward OFF (RPWM2 = PB5)
        TIM3->CCR1 = pwm_right;             // Reverse ON (LPWM2 = PB4)
    }
    else {
        // BRAKE
        GPIOB->BRR = (1 << 3);
        GPIOD->BRR = (1 << 2);
        TIM3->CCR2 = 0;
        TIM3->CCR1 = 0;
    }
}
