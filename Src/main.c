#include "stm32f051x8.h"
#include <stdint.h>
#include "line_sensors.h"
#include "motors.h"

int main(void)
{
    // 1. Hardware Initialization
    Sensors_Init();
    Motors_Init();

    // Memory variable. It starts assuming it's centered
    int last_state = STRAIGHT_LINE;

    // Infinite control loop
    while(1) {
        // Update sensor readings
    	actual_reading = Sensor_Lecture();

        // ---------- NAVIGATION LOGIC ----------

        // 1. PIT STOP / INTERSECTION (Highest Priority)
        if (actual_reading == CROSS_LINE) {
            // Stop both motors immediately
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

        // 2. NORMAL NAVIGATION
        else if (actual_reading == STRAIGHT_LINE) {
            // Both motors go forward
            Set_Motor_Speed(200, 200);

            // Memory save at center
            last_state = STRAIGHT_LINE;
        }
        else if (actual_reading == LEFT_LINE) {
            // Right motor on, left motor slower = Turn Left
            Set_Motor_Speed(40, 200);

            // Memory save, last turn
            last_state = LEFT_LINE;
        }
        else if (actual_reading == RIGHT_LINE) {
            // Left motor on, Right motor slower = Turn Right
            Set_Motor_Speed(200, 40);

            // Memory save, last turn
            last_state = RIGHT_LINE;
        }

        // ---------- LOST LINE (EMERGENCY) ----------
        else if (actual_reading == LOST_LINE) {
            // Line is lost, so we CHECK MEMORY

            if (last_state == STRAIGHT_LINE){
                // We are fine, we keep going forward trying to find it
                Set_Motor_Speed(0, 0);
            }
            else if (last_state == LEFT_LINE) {
                // We lost the line going left, keep turning left
                Set_Motor_Speed(40, 200);
            }
            else if (last_state == RIGHT_LINE) {
                // We lost the line going right, keep turning right
                Set_Motor_Speed(200, 40);
            }
        }
    }
}
