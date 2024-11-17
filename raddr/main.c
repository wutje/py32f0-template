//PLAN: hook up switch to irq, toggle led

#include "py32f0xx_bsp_clock.h"
#include "py32f0xx_hal_exti.h"

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

void cfg_pin(uint32_t pin, uint32_t mode, uint32_t pull)
{
    GPIO_InitTypeDef pin_cfg;

    pin_cfg.Pin   = pin;
    pin_cfg.Mode  = mode;
    pin_cfg.Pull  = pull;
    pin_cfg.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &pin_cfg);
}

static void cfg_gpio(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    cfg_pin(SWC_PIN,     GPIO_MODE_IT_FALLING, GPIO_PULLUP);
    cfg_pin(KEY_IN_PIN,  GPIO_MODE_INPUT,      GPIO_PULLUP);
    cfg_pin(KEY_OUT_PIN, GPIO_MODE_OUTPUT_PP,  GPIO_NOPULL);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void EXTI0_1_IRQHandler(void)
{
    GPIO_PinState switch_in;
    switch_in = HAL_GPIO_ReadPin(GPIOA, SWC_PIN);
    HAL_GPIO_WritePin(GPIOA, KEY_OUT_PIN, switch_in);
}


int main(void)
{
    HAL_Init();
    BSP_HSI_24MHzClockConfig();
    cfg_gpio();

    GPIO_PinState switch_in, key_in, key_out;

    while (1);
    if(0){
        /*// Inverteer alle inputs zodat ik mijn logica blijf snappen*/
        /*switch_in = !HAL_GPIO_ReadPin(GPIOA, SWC_PIN);*/
        /*key_in    = !HAL_GPIO_ReadPin(GPIOA, KEY_IN_PIN);*/

        /*[>key_out = switch_in | key_in;<]*/
        /*key_out = switch_in;*/

        /*// Inverteer alle outputs zodat ik mijn logica blijf snappen*/
        /*// (ledje hangt tussen vcc en key_out)*/
        /*HAL_GPIO_WritePin(GPIOA, KEY_OUT_PIN, !key_out);*/
    }
}
