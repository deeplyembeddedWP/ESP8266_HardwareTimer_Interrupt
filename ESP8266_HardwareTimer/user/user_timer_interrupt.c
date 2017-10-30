#include "eagle_soc.h"
#include "ets_sys.h"
#include "osapi.h"
#include "user_MyConf.h"

/* Timer Interrupt Values */
#define TIMER_INTERRUPT_ONE_US 150 /* Initialize hw timer to trigger every 150us */
#define TIMER_INTERRUPT_TWO_US 250 /* Initialize hw timer to trigger every 250us */
#define NMI_ISR_SOURCE 1           /* Initialize hw timer as NMI (Nothing can inturrpt the ISR)*/

/* Test Pin */
#define GPIO_PIN_4 4
#define PIN_HIGH 1

/* Arm timer to trigger every 150us */
#define SET_150_US

/*********************************************************
 * Function Name   : Toggle_GPIO
 * Description     : Toggles the GPIO_04 every x usec
 * Returns         : NONE
 * Params          : NONE
 ********************************************************/
static void Toggle_GPIO()
{
    unsigned char pin_status;
    pin_status = digitalRead(GPIO_PIN_4);
    if (pin_status == PIN_HIGH)
        digitalWrite(GPIO_PIN_4, LOW);
    else
        digitalWrite(GPIO_PIN_4, HIGH);
}

/*********************************************************
 * Function Name : Initialize_ISR
 * Description   : Initializes the GPIO_04, registers
                   the ISR and arms the timer.
 * Returns       : NONE
 * Params        : NONE
 ********************************************************/
void ICACHE_FLASH_ATTR Initialize_ISR()
{
    /* Configure and  Initialize the GPIO_4 */
    pinMode(GPIO_PIN_4, OUTPUT);
    digitalWrite(GPIO_PIN_4, LOW);

    /* Initialize the hw Timer for generating Non-Maskable Interrupts (NMI)*/
    hw_timer_init(NMI_ISR_SOURCE, 1);

    /* Register the ISR to run every time the interrupt occurs */
    hw_timer_set_func(Toggle_GPIO);

/* Arm the timer to trigger every x microseconds */
#ifdef SET_150_US
    hw_timer_arm(TIMER_INTERRUPT_ONE_US);
#else
    hw_timer_arm(TIMER_INTERRUPT_TWO_US);
#endif
}

