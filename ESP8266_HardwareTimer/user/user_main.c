#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_MyConf.h"

void
user_rf_pre_init (void)
{
}

/*Init function*/
void ICACHE_FLASH_ATTR
user_init ()
{
  /* Initialize and Trigger the Timer Interrupt*/
  Initialize_ISR();
  
}
