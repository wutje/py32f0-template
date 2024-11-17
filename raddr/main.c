/***
 * Demo: LED Toggle
 *
 * PA0   ------> LED+
 * GND   ------> LED-
 */
//#include "py32f0xx_bsp_printf.h"
#include "py32f0xx_bsp_clock.h"

/*  A wolf is:
 *  PA1 = switch (Pin 7)
 *  PA2 = shared with reset (Pin 6)
 *  PA13/PB3/SWDIO = SWD (Pin 5)
 *  PA14/PB3/SWCLK = SWC (Pin 4)
 *  PA3 = KEY Data in (Pin 3)
 *  PA4 = KEY Data out (Pin 2)
 *  VCC = (Pin 1)
 *
 * */

#define SWC_PIN     GPIO_PIN_1
#define KEY_IN_PIN  GPIO_PIN_3
#define KEY_OUT_PIN GPIO_PIN_4

static void APP_GPIO_Config(void);

int main(void)
{
    HAL_Init();
    BSP_HSI_24MHzClockConfig();
    APP_GPIO_Config();

    GPIO_PinState switch_in, key_in, key_out;

    while (1)
    {
        // Inverteer alle inputs zodat ik mijn logica blijf snappen
        switch_in = !HAL_GPIO_ReadPin(GPIOA, SWC_PIN);
        key_in    = !HAL_GPIO_ReadPin(GPIOA, KEY_IN_PIN);

        key_out = switch_in | key_in;

        // Inverteer alle outputs zodat ik mijn logica blijf snappen
        // (ledje hangt tussen vcc en key_out)
        HAL_GPIO_WritePin(GPIOA, KEY_OUT_PIN, !key_out);
    }
}

static void APP_GPIO_Config(void)
{
    GPIO_InitTypeDef cfg_key_out, cfg_key_in, cfg_switch;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /*PA1, SWITCH*/
    cfg_switch.Pin   = SWC_PIN;
    cfg_switch.Mode  = GPIO_MODE_INPUT;
    cfg_switch.Pull  = GPIO_PULLUP;
    cfg_switch.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &cfg_switch);
    /*PA3,  KEY IN*/
    cfg_key_in.Pin   = KEY_IN_PIN;
    cfg_key_in.Mode  = GPIO_MODE_INPUT;
    cfg_key_in.Pull  = GPIO_PULLUP;
    cfg_key_in.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &cfg_key_in);
    /*PA4, KEY OUT*/
    cfg_key_out.Pin   = KEY_OUT_PIN;
    cfg_key_out.Mode  = GPIO_MODE_OUTPUT_PP;
    cfg_key_out.Pull  = GPIO_PULLUP;
    cfg_key_out.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &cfg_key_out);
}

void APP_ErrorHandler(void)
{
    while (1);
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Export assert error source and line number
  */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    while (1);
}
#endif /* USE_FULL_ASSERT */
