/******************************************************************************
* Copyright 2013-2014 Espressif Systems (Wuxi)
*
* FileName: hw_time.c
*
* Description: hw_time driver
*
* Modification history:
*     2014/5/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#if 1

#define US_TO_RTC_TIMER_TICKS(t)          \
    ((t) ?                                   \
     (((t) > 0x35A) ?                   \
      (((t)>>2) * ((APB_CLK_FREQ>>4)/250000) + ((t)&0x3) * ((APB_CLK_FREQ>>4)/1000000))  :    \
      (((t) *(APB_CLK_FREQ>>4)) / 1000000)) :    \
     0)

#define FRC1_ENABLE_TIMER  BIT7
#define FRC1_AUTO_LOAD  BIT6

//TIMER PREDIVED MODE
typedef enum {
    DIVDED_BY_1 = 0,		//timer clock
    DIVDED_BY_16 = 4,	//divided by 16
    DIVDED_BY_256 = 8,	//divided by 256
} TIMER_PREDIVED_MODE;

typedef enum {			//timer interrupt mode
    TM_LEVEL_INT = 1,	// level interrupt
    TM_EDGE_INT   = 0,	//edge interrupt
} TIMER_INT_MODE;

typedef enum {
    FRC1_SOURCE = 0,	
    NMI_SOURCE = 1,	
} FRC1_TIMER_SOURCE_TYPE;

static void (* user_hw_timer_cb)(void) = NULL;
static u8 TimeType = 0;
static u8 AutoLoad = 0;
static u8 Cando = 0;
static void  hw_timer_isr_cb(void)
{
        //if(user_hw_timer_cb!=NULL)
               // (*(user_hw_timer_cb))();

         if(AutoLoad == 1)
        {
                if (user_hw_timer_cb != NULL) {
                    (*user_hw_timer_cb)();
                }
        }
        else if(AutoLoad == 0)
        {
                if(Cando)
                {
                        Cando = 0;  
                        if (user_hw_timer_cb != NULL) {
                            (*user_hw_timer_cb)();
                        }
                }
        }
}



/******************************************************************************
* FunctionName : hw_timer_arm
* Description  : set a trigger timer delay for this timer.
* Parameters   : uint32 val : in autoload mode
                        50 ~ 0x7fffff;  for FRC1 source.
                        100 ~ 0x7fffff;  for NMI source.
in non autoload mode:
                        10 ~ 0x7fffff;  
* Returns      : NONE
*******************************************************************************/
void  hw_timer_arm(u32 val)
{
      RTC_REG_WRITE(FRC1_LOAD_ADDRESS, US_TO_RTC_TIMER_TICKS(val));
       if(AutoLoad == 0)
       {
              Cando = 1;
       }
}


/******************************************************************************
* FunctionName : hw_timer_set_func
* Description  : set the func, when trigger timer is up.
* Parameters   : void (* user_hw_timer_cb_set)(void):
                        timer callback function,
* Returns      : NONE
*******************************************************************************/
void  hw_timer_set_func(void (* user_hw_timer_cb_set)(void))
{
        user_hw_timer_cb = user_hw_timer_cb_set;
}



/******************************************************************************
* FunctionName : hw_timer_init
* Description  : initilize the hardware isr timer
* Parameters   : 
FRC1_TIMER_SOURCE_TYPE source_type:
                        FRC1_SOURCE,    timer use frc1 isr as isr source. 
                        NMI_SOURCE,     timer use nmi isr as isr source. 
u8 req:
                        0,  not autoload,
                        1,  autoload mode,
* Returns      : NONE
*******************************************************************************/
void ICACHE_FLASH_ATTR hw_timer_init(FRC1_TIMER_SOURCE_TYPE source_type,u8 req)
{
        if(req == 1){
                RTC_REG_WRITE(FRC1_CTRL_ADDRESS,    FRC1_AUTO_LOAD|
                              DIVDED_BY_16
                              | FRC1_ENABLE_TIMER
                              | TM_EDGE_INT);
                AutoLoad = 1;
        }
        else{
                RTC_REG_WRITE(FRC1_CTRL_ADDRESS,
                          DIVDED_BY_16
                          | FRC1_ENABLE_TIMER
                          | TM_EDGE_INT);
                AutoLoad = 0;
        }
        
        if(source_type == NMI_SOURCE)
        {
              // #define NMI_SOURCE_SEL_REG 0x3ff00000
              // WRITE_PERI_REG(NMI_SOURCE_SEL_REG, (READ_PERI_REG(NMI_SOURCE_SEL_REG)&~0x1F)|(0x1+0x7*2) );
              // NmiTimSetFunc(hw_timer_isr_cb);
               ETS_FRC_TIMER1_NMI_INTR_ATTACH(hw_timer_isr_cb);
               TimeType = 1;
        }
        else{
                ETS_FRC_TIMER1_INTR_ATTACH( hw_timer_isr_cb,NULL);
                TimeType = 0;
        }
        
        TM1_EDGE_INT_ENABLE();
        ETS_FRC1_INTR_ENABLE();
}


void hw_timer_stop(void)
{
//RTC_REG_WRITE(FRC1_CTRL_ADDRESS,DIVDED_BY_16 | FRC1_ENABLE_TIMER | TM_EDGE_INT);
	os_printf("hw tim stop\n");
  //  RemoveNmiISR();
    hw_timer_init(1, 0);
   // hw_timer_set_func(NULL);
    //hw_timer_arm(150);
    
}

//-------------------------------Test Code Below--------------------------------------
#if 0

Example1: 
#define REG_READ(_r) (*(volatile uint32 *)(_r))
#define WDEV_NOW()\
    REG_READ(0x3ff20c00)
static u32 tick_now2 = 0;
void   hw_test_timer_cb(void)
{
    static uint16 j = 0;
    j++;
    
    if( (WDEV_NOW() - tick_now2) >= 1000000 )
    {
    static u32 idx = 1; 
        tick_now2 = WDEV_NOW();
        os_printf("b%u:%d\n",idx++,j);
        j = 0;
    }
    //hw_timer_arm(50);
}

typedef enum {
    FRC1_SOURCE = 0,   
    NMI_SOURCE = 1,   
} FRC1_TIMER_SOURCE_TYPE;

void ICACHE_FLASH_ATTR user_init(void)
{
        hw_timer_init(FRC1_SOURCE,1);   //hw_timer_init(NMI_SOURCE,1);
        hw_timer_set_func(hw_test_timer_cb);
        hw_timer_arm(100);
}


Example2: 
/*
Do not use ICACHE_FLASH_ATTR before this ISR Callback
static void ICACHE_FLASH_ATTR stepper_timer_cb(void)  is WRONG
*/
static void  stepper_timer_cb(void)
{
    static u16  j=100;
    if(j-- ==0)
        {ets_printf("z%u\n",j);  j=100;}
    hw_timer_arm(2500);
}

void ICACHE_FLASH_ATTR user_init(void)
{
    hw_timer_init(0,0);
    hw_timer_set_func(stepper_timer_cb);
    hw_timer_arm(2500);
}

#endif
/*
NOTE:
1 if use nmi source, for autoload timer , the timer setting val can't be less than 100. 
2 if use nmi source, this timer has highest priority, can interrupt other isr.
3 if use frc1 source, this timer can't interrupt other isr.

*/

#endif
