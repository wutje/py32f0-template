//PLAN: hook up switch to irq, toggle led

#include "py32f0xx_bsp_clock.h"

#include "wolf.h"
#include "pack.h"

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

    cfg_pin(SWC_PIN,     GPIO_MODE_INPUT,      GPIO_PULLUP);
    cfg_pin(KEY_IN_PIN,  GPIO_MODE_IT_FALLING, GPIO_PULLUP);
    cfg_pin(KEY_OUT_PIN, GPIO_MODE_OUTPUT_PP,  GPIO_NOPULL);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}
void gpio_set(int bit)
{
    HAL_GPIO_WritePin(GPIOA, KEY_OUT_PIN, bit);
}

void sleep_ns(int delay)
{
    HAL_Delay(delay); //TODO these are ms
}

int gpio_get(void)
{
    return HAL_GPIO_ReadPin(GPIOA, KEY_IN_PIN);
}

bool SOME_INPUT[K] = {0,0,0,0, 0,0,0,0};

void update_input(void)
{
    SOME_INPUT[0] = HAL_GPIO_ReadPin(GPIOA, SWC_PIN);
}

void EXTI2_3_IRQHandler(void)
{
    interrupt_rising();
}

int main(void)
{
    HAL_Init();
    BSP_HSI_24MHzClockConfig();
    cfg_gpio();

    while (1);
}
