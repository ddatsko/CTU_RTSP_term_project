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

volatile unsigned irq_count;


/*!
 * \brief Motor interrupt handler
 * Interrupt handler for handling intrrupts on the stepper motor phase change
 */
void irc_isr(void)
{
        bool irc_a = (MOTOR_SR & BIT(MOTOR_SR_IRC_A_MON)) != 0;
        bool irc_b = (MOTOR_SR & BIT(MOTOR_SR_IRC_B_MON)) != 0;
        // ...
        irq_count++;
        GPIO_INT_STATUS = MOTOR_IRQ_PIN; /* clear the interrupt */
}

void irc_init(void)
{
        GPIO_INT_STATUS = MOTOR_IRQ_PIN; /* reset status */
        GPIO_DIRM = 0x0;                 /* set as input */
        GPIO_INT_TYPE = MOTOR_IRQ_PIN;   /* interrupt on edge */
        GPIO_INT_POLARITY = 0x0;         /* falling edge */
        GPIO_INT_ANY = 0x0;              /* ignore rising edge */
        GPIO_INT_ENABLE = MOTOR_IRQ_PIN; /* enable interrupt on MOTOR_IRQ pin */

        intConnect(INUM_TO_IVEC(INT_LVL_GPIO), irc_isr, 0);
        intEnable(INT_LVL_GPIO);         /* enable all GPIO interrupts */
}

void irc_cleanup(void)
{
        GPIO_INT_DISABLE = MOTOR_IRQ_PIN;

        intDisable(INT_LVL_GPIO);
        intDisconnect(INUM_TO_IVEC(INT_LVL_GPIO), irc_isr, 0);
}

/*
 * Entry point for DKM.
 */
void motor(void)
{

        irc_init();
        
        // Motor testing
        PWM_PERIOD_REGISTER = 5000;
        PWM_DUTY_CR = ((1 << DUTY_DIR_R) | (200 << DUTY_CYCLE));
        
        MOTOR_CR = (1 << MOTOR_CR_PWM_ENABLE);
        
        

        while (1) {
            printf("IRQ count: %u\n", irq_count);
            int i;
            sleep(1);
        }

        irc_cleanup();
}
