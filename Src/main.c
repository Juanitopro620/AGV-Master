#include "stm32f051x8.h"
#include <stdint.h>
#include "line_sensors.h"
#include "motors.h"
#include "encoders.h" // Added: Includes your teammate's encoder library

// Global variables for the Closed-Loop Speed Controller (PI Accumulators)
float pwm_left_current = 0.0f;
float pwm_right_current = 0.0f;

/**
 * @brief Closed-Loop Speed Controller (PI Loop)
 * Adjusts PWM dynamically to maintain constant RPM regardless of load/weight.
 */
void Controlar_RPM(float target_left_rpm, float target_right_rpm) {
    // 1. Calculate the mathematical error
    float error_left = target_left_rpm - velocidad_izq_rpm;
    float error_right = target_right_rpm - velocidad_der_rpm;

    // 2. Integral Gain (Ki): Adjusts how aggressively the AGV reacts to weight/friction
    float Ki = 0.4f; 

    // 3. Accumulate power (If weight increases, error accumulates to push harder)
    pwm_left_current += (error_left * Ki);
    pwm_right_current += (error_right * Ki);

    // 4. Clamping safety limits (ARR top limit is 799)
    if (pwm_left_current > 799.0f)  pwm_left_current = 799.0f;
    if (pwm_left_current < 0.0f)    pwm_left_current = 0.0f;
    if (pwm_right_current > 799.0f) pwm_right_current = 799.0f;
    if (pwm_right_current < 0.0f)   pwm_right_current = 0.0f;

    // 5. Send the calculated duty cycle to the hardware registers
    Set_Motor_Speed((int)pwm_left_current, (int)pwm_right_current);
}

int main(void)
{
    // 1. Hardware Initialization
    Sensors_Init();
    Motors_Init();
    Encoders_Init(); // Added: Initializes Port C EXTI Interrupts for encoders
    SysTick_Init();  // Added: Configures the 10ms system timer for RPM updates

    // Memory variable. It starts assuming it's centered
    int last_state = STRAIGHT_LINE;

    // AGV SPEED LIMITS (Target values in RPM)
    // Adjust these values to find the perfect balance for your track
    float rpm_fast = 90.0f; 
    float rpm_slow = 25.0f;

    // Infinite control loop
    while(1) {
        // Update sensor readings
    	actual_reading = Sensor_Lecture();

        // ---------- NAVIGATION LOGIC ----------

        // 1. PIT STOP / INTERSECTION (Highest Priority)
        if (actual_reading == CROSS_LINE) {
            // Reset speed accumulators and stop both motors immediately
            pwm_left_current = 0.0f;
            pwm_right_current = 0.0f;
            Set_Motor_Speed(0, 0);

            // Smart wait loop:
            // Stay frozen here until the robot ONLY sees the center line
            while (actual_reading != STRAIGHT_LINE) {
                // We must keep reading the sensors inside this loop
                // to know when the robot has been pushed/aligned manually
                actual_reading = Sensor_Lecture();
            }

            // Once the loop breaks, we update memory just in case
            last_state = STRAIGHT_LINE;
        }

        // 2. NORMAL NAVIGATION (Closed-Loop Controlled)
        else if (actual_reading == STRAIGHT_LINE) {
            // Go straight keeping both wheels at maximum straight speed
            Controlar_RPM(rpm_fast, rpm_fast);
            last_state = STRAIGHT_LINE;
        }
        else if (actual_reading == LEFT_LINE) {
            // Turn Left: Slow down left wheel, speed up right wheel
            Controlar_RPM(rpm_slow, rpm_fast);
            last_state = LEFT_LINE;
        }
        else if (actual_reading == RIGHT_LINE) {
            // Turn Right: Speed up left wheel, slow down right wheel
            Controlar_RPM(rpm_fast, rpm_slow);
            last_state = RIGHT_LINE;
        }

        // ---------- LOST LINE (EMERGENCY) ----------
        else if (actual_reading == LOST_LINE) {
            // Line is lost, so we CHECK MEMORY

            if (last_state == STRAIGHT_LINE){
                // Safe stop if line was lost while going straight
                pwm_left_current = 0.0f;
                pwm_right_current = 0.0f;
                Set_Motor_Speed(0, 0);
            }
            else if (last_state == LEFT_LINE) {
                // Keep turning left using controlled RPM
                Controlar_RPM(rpm_slow, rpm_fast);
            }
            else if (last_state == RIGHT_LINE) {
                // Keep turning right using controlled RPM
                Controlar_RPM(rpm_fast, rpm_slow);
            }
        }
    }
}
