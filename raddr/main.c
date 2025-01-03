#include <py32f0xx_hal.h>
#include "clk_config.h"

#include "uid.h"
#include "output_timer.h"
#include "wolf.h"
#include "pack.h"

/*  A wolf is:
 *  Pin     Port(s)         PCB function    SPI1        I2C     UART1       TIM1        Alternate functions
 *  Pin 8   GND             GND             -
 *  Pin 7   PA1             switch          SCK/MOSI            RTS         CH4 / CH2N
 *  Pin 6   PA2/PF2         reset           SCK/MOSI    SDA     TX                      COMP2_OUT (PF2: RESET/MCO)
 *  Pin 5   PA13            SWD             MISO                RX          CH2         SWDIO
 *  Pin 4   PA14/PB3        SWC             SCK                 TX / RTS    CH2         SWCLK / MCO
 *  Pin 3   PA3             KEY Data inp    MOSI        SCL     RX          CH1
 *  Pin 2   PA4/PA10        KEY Data out    NSS         SDA/SCL CK /RX /TX  CH3
 *  Pin 1   VCC             VCC             -
 * */

#define SWC_PIN     GPIO_PIN_1
#define KEY_IN_PIN  GPIO_PIN_3
#define KEY_OUT_PIN GPIO_PIN_4

volatile int data_ready;

/**
 * These functions need to be implemented by us, They are called
 * by the pack.c code
 */
/* start */
int gpio_get(void)
{
    return HAL_GPIO_ReadPin(GPIOA, KEY_IN_PIN);
}

void gpio_set(int bit)
{
    HAL_GPIO_WritePin(GPIOA, KEY_OUT_PIN, bit);
}

void sleep_ns(int delay_ns)
{
    HAL_Delay(delay_ns); //TODO these are ms
}

bool K_BINARY_INPUTS[K] = {0};

void update_input(void)
{
    // We update the in the switch interrupt handler so this is a NO-OP
}
/* end */


void cfg_pin(uint32_t pin, uint32_t mode, uint32_t pull)
{
    GPIO_InitTypeDef pin_cfg;

    pin_cfg.Pin   = pin;
    pin_cfg.Mode  = mode;
    pin_cfg.Pull  = pull;
    pin_cfg.Speed = GPIO_SPEED_FREQ_HIGH;
    //MEGA HACK SMELLY CODE.
    //We only specify Alternate Function for one pin. and so this works.
    //Do not do try this at home.
    pin_cfg.Alternate = GPIO_AF13_TIM1;
    HAL_GPIO_Init(GPIOA, &pin_cfg);
}

static void cfg_gpio(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    cfg_pin(SWC_PIN,     GPIO_MODE_IT_RISING_FALLING, GPIO_PULLUP);
    cfg_pin(KEY_IN_PIN,  GPIO_MODE_AF_OD, GPIO_PULLUP);
    cfg_pin(KEY_OUT_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

    /* EXTI interrupt init*/
    /*HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);*/
    /*HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);*/

    HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
}

/**
 * Switch interrupt handler
 * Rising and falling edges
 */
void EXTI0_1_IRQHandler(void)
{
    //I think this causes bounce issues
    K_BINARY_INPUTS[0] = !HAL_GPIO_ReadPin(GPIOA, SWC_PIN);
    __HAL_GPIO_EXTI_CLEAR_IT(SWC_PIN);
}

/**
 * Key in interrupt handler
 * Falling edges
 */
void EXTI2_3_IRQHandler(void)
{
    data_ready = 1;
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_IN_PIN);
}

int main(void)
{
    /* Setup clock BEFORE HAL_Init.
     * And before running anything else */
    BSP_HSI_24MHzClockConfig();
    HAL_Init();
#if defined USE_SEMIHOSTING
    {
        //This 'magic' function needs to be called.
        //libgloss will setup I/O structures to use
        extern void initialise_monitor_handles();
        initialise_monitor_handles();
        printf("PY32F0xx\r\nSystem Clock: %ld\r\n", SystemCoreClock);
        uid_print();
    }
#endif

    cfg_gpio();
    raddr_output_init();

    uint32_t t_last_call = 0;

    /*
    // if switch pressed at boot become AKELA.
    int akela = !HAL_GPIO_ReadPin(GPIOA, SWC_PIN);

    if (akela) while (1) {
        rally_pack(); //wakeup pack and bark own state
        // Stop Yapping for a while
#define W 9
        sleep_ns((T1H+T1L+STFU)*(W*K+W+3) + 100);
#undef W
    }
    */

    uint32_t t_bounce = 0;
    while (1) {
        uint32_t now = HAL_GetTick();

        if (!data_ready) {
            // if no data, take the time to update inputs
            bool input = !HAL_GPIO_ReadPin(GPIOA, SWC_PIN);
            // if input changed only set it when done bouncing
            if (input^K_BINARY_INPUTS[0] && t_bounce < now) {
                K_BINARY_INPUTS[0] = input;
                t_bounce = now + 2; // only accept change in 2ms
            }
            continue;
        }

        sleep_ns((T0H+T1H)/2);
        int bit = gpio_get();

        data_ready = 0;

        // The statemachine needs to know the elapsed time so
        // it can reset when out of sync.
        join_cry(bit, now - t_last_call);
        t_last_call = now;
    }
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
