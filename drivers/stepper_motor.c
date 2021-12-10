#include "stepper_motor.h"
#include <taskLib.h>
#include <stdio.h>
#include <kernelLib.h>
#include <semLib.h>
#include <intLib.h>
#include <iv.h>
#include <xlnx_zynq7k.h>
#include <stdbool.h>
#include "../shm.h"

#define TRANSITION(old, new) (((old) << 2) | new)
static const int TRANSITION_TABLE[16] = {
          [TRANSITION(0,1)] = +1,
          [TRANSITION(1,3)] = +1,
          [TRANSITION(3,2)] = +1,
          [TRANSITION(2,0)] = +1,
  
          [TRANSITION(1,0)] = -1,
          [TRANSITION(3,1)] = -1,
          [TRANSITION(2,3)] = -1,
          [TRANSITION(0,2)] = -1,
};

static SEM_ID sem_motor_pwm_change;
static motor_driver_shared_memory_t *shm;
static uint8_t motor_state, prev_motor_state;

/*!
 * \brief Set duty cycle in percentage
 * Set duty cycle for controlling the motor to the values from -100 to 100,
 * where -100 means max speed backwards, 100 means max speed forward, 0 means no movement and so on
 * @param duty_cycle_percent : desired percentage of duty cycle (from -100 to 100)
 */
inline void set_motor_dutycycle(int8_t duty_cycle_percent) 
{
	if (duty_cycle_percent < -100 || duty_cycle_percent > 100) 
	{
		return;
	}
	if (duty_cycle_percent < 0) 
	{
		PWM_DUTY_CR = ((1 << DUTY_DIR_R) | (((PWM_PERIOD * -duty_cycle_percent) / 100) << DUTY_CYCLE));
	} 
	else 
	{
		PWM_DUTY_CR = ((1 << DUTY_DIR_F) | (((PWM_PERIOD * duty_cycle_percent) / 100) << DUTY_CYCLE));
	}
}

/*!
 * \brief Motor interrupt handler
 * Interrupt handler for handling intrrupts on the stepper motor phase change
 */
void motor_isr(void)
{
        bool irc_a = (MOTOR_SR & BIT(MOTOR_SR_IRC_A_MON)) != 0;
        bool irc_b = (MOTOR_SR & BIT(MOTOR_SR_IRC_B_MON)) != 0;
        
        prev_motor_state = motor_state;
        motor_state = irc_a | (irc_b << 1);
        shm->position += TRANSITION_TABLE[TRANSITION(prev_motor_state, motor_state)];
                
        
        shm->irq_count++;
        GPIO_INT_STATUS = MOTOR_IRQ_PIN; /* clear the interrupt */
        
        // TODO: check when the semapore should be given and give it only in that situations
//        if (shm->desired_position - shm->position == 0 || ) {
        semGive(sem_motor_pwm_change);
//		  }
}

/*!
 * \brief Initialize the step motor
 */
void motor_init(void)
{
        GPIO_INT_STATUS = MOTOR_IRQ_PIN; /* reset status */
        GPIO_DIRM = 0x0;                 /* set as input */
        GPIO_INT_TYPE = MOTOR_IRQ_PIN;   /* interrupt on edge */
        GPIO_INT_POLARITY = 0x0;         /* falling edge */
        GPIO_INT_ANY = 0x0;              /* ignore rising edge */
        GPIO_INT_ENABLE = MOTOR_IRQ_PIN; /* enable interrupt on MOTOR_IRQ pin */
        
        PWM_PERIOD_REGISTER = PWM_PERIOD; // Set PWM period to 20 kHz 
        MOTOR_CR = (1 << MOTOR_CR_PWM_ENABLE); // Enable PWM by default
        PWM_DUTY_CR = 0; // No PWM for now
        
        bool irc_a = (MOTOR_SR & BIT(MOTOR_SR_IRC_A_MON)) != 0;
        bool irc_b = (MOTOR_SR & BIT(MOTOR_SR_IRC_B_MON)) != 0;
        
        // TODO: check if this is ok with bools here
        motor_state =  irc_a | (irc_b << 1);
     
        intConnect(INUM_TO_IVEC(INT_LVL_GPIO), motor_isr, 0);
        intEnable(INT_LVL_GPIO);         /* enable all GPIO interrupts */
}

void motor_cleanup(void)
{
        GPIO_INT_DISABLE = MOTOR_IRQ_PIN;

        intDisable(INT_LVL_GPIO);
        intDisconnect(INUM_TO_IVEC(INT_LVL_GPIO), motor_isr, 0);
}

/*!
 * \brief Entry point to the DKM
 */
void motor(void)
{        
        // Init semaphore for changing PWM duty requests
        sem_motor_pwm_change = semOpen("/sem_motor_pwm_change", SEM_TYPE_COUNTING, SEM_EMPTY, SEM_Q_FIFO, OM_CREATE, NULL);
        if (sem_motor_pwm_change == SEM_ID_NULL) 
        {
        	perror("PWM change semaphore");
        	motor_cleanup();
        	return;
        }
        
        // Init shared memory
        shm = init_shm();
        if (shm == NULL) 
        {
        	perror("Shared motor driver shared memory");
        	motor_cleanup();
        	semClose(sem_motor_pwm_change);
        	return;
        }
        
        // Initialize motor control
        motor_init();
        
        int32_t movement;
        int8_t new_duty_cycle;
        while (1)
        {
        	// Wait for the request to recalculate the duty cycle
            semTake(sem_motor_pwm_change, WAIT_FOREVER);
            
            // TODO: check this and modify to make everything more smoothly
            movement = shm->desired_position - shm->position;
            if (movement < -100) {
            	new_duty_cycle = -50;
            } else if (movement > 100) {
            	new_duty_cycle = 50;
            } else if (movement > 0) {
            	new_duty_cycle = 10;
            } else if (movement < 0) {
            	new_duty_cycle = -10;
            } else {
            	new_duty_cycle = 0;
            }
            set_motor_dutycycle(new_duty_cycle);
        }
        
        semClose(sem_motor_pwm_change);
        motor_cleanup();
}
