#include "c_types.h"
#include "eagle_soc.h"
#include "ets_sys.h"
#include "user_esp8266_peri.h"
#include "user_GPIO_Config.h"

void ICACHE_FLASH_ATTR pinMode(uint8_t pin, uint8_t mode);
void ICACHE_FLASH_ATTR digitalWrite(uint8_t pin, uint8_t val);
int ICACHE_FLASH_ATTR digitalRead(uint8_t pin);
 
uint8_t esp8266_gpioToFn[16] = {0x34, 0x18, 0x38, 0x14, 0x3C, 0x40, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x04, 0x08, 0x0C, 0x10};

void ICACHE_FLASH_ATTR pinMode(uint8_t pin, uint8_t mode) 
{
  if(pin < 16)
  {
    if(mode == SPECIAL)
	{
      GPC(pin) = (GPC(pin) & (0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
      GPEC = (1 << pin); //Disable
      GPF(pin) = GPFFS(GPFFS_BUS(pin));//Set mode to BUS (RX0, TX0, TX1, SPI, HSPI or CLK depending in the pin)
      if(pin == 3) GPF(pin) |= (1 << GPFPU);//enable pullup on RX
    } 
	else if(mode & FUNCTION_0)
	{
      GPC(pin) = (GPC(pin) & (0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
      GPEC = (1 << pin); //Disable
      GPF(pin) = GPFFS((mode >> 4) & 0x07);
      if(pin == 13 && mode == FUNCTION_4) GPF(pin) |= (1 << GPFPU);//enable pullup on RX
    }  
	else if(mode == OUTPUT || mode == OUTPUT_OPEN_DRAIN)
	{
      GPF(pin) = GPFFS(GPFFS_GPIO(pin));//Set mode to GPIO
      GPC(pin) = (GPC(pin) & (0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
      if(mode == OUTPUT_OPEN_DRAIN) GPC(pin) |= (1 << GPCD);
      GPES = (1 << pin); //Enable
    } 
	else if(mode == INPUT || mode == INPUT_PULLUP)
	{
      GPF(pin) = GPFFS(GPFFS_GPIO(pin));//Set mode to GPIO
      GPEC = (1 << pin); //Disable
      GPC(pin) = (GPC(pin) & (0xF << GPCI)) | (1 << GPCD); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
      if(mode == INPUT_PULLUP) 
	  {
          GPF(pin) |= (1 << GPFPU);  // Enable  Pullup
      }
    } 
	else if(mode == WAKEUP_PULLUP || mode == WAKEUP_PULLDOWN)
	{
      GPF(pin) = GPFFS(GPFFS_GPIO(pin));//Set mode to GPIO
      GPEC = (1 << pin); //Disable
      if(mode == WAKEUP_PULLUP) 
	  {
          GPF(pin) |= (1 << GPFPU);  // Enable  Pullup
          GPC(pin) = (1 << GPCD) | (4 << GPCI) | (1 << GPCWE); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(LOW) | WAKEUP_ENABLE(ENABLED)
      } 
	  else 
	  {
          GPF(pin) |= (1 << GPFPD);  // Enable  Pulldown
          GPC(pin) = (1 << GPCD) | (5 << GPCI) | (1 << GPCWE); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(HIGH) | WAKEUP_ENABLE(ENABLED)
      }
    }
  } 
  else if(pin == 16)
  {
    GPF16 = GP16FFS(GPFFS_GPIO(pin));//Set mode to GPIO
    GPC16 = 0;
    if(mode == INPUT || mode == INPUT_PULLDOWN_16)
	{
      if(mode == INPUT_PULLDOWN_16)
	  {
        GPF16 |= (1 << GP16FPD);//Enable Pulldown
      }
      GP16E &= ~1;
    } 
	else if(mode == OUTPUT)
	{
      GP16E |= 1;
    }
  }
}

void ICACHE_FLASH_ATTR digitalWrite(uint8_t pin, uint8_t val) 
{
  if(pin < 16)
  {
    if(val) 
		GPOS = (1 << pin);
    else 
		GPOC = (1 << pin);
  } else if(pin == 16)
  {
    if(val) 
		GP16O |= 1;
    else 
		GP16O &= ~1;
  }
}

int ICACHE_FLASH_ATTR digitalRead(uint8_t pin) 
{
  if(pin < 16)
  {
    return GPIP(pin);
  } else if(pin == 16)
  {
    return GP16I & 0x01;
  }
  return 0;
}
