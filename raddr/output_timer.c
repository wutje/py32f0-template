#include <stdio.h>
#include <stdbool.h>
#include <py32f0xx_hal.h>
#include "wolf.h"
#include "output_timer.h"
//
//TODO get these from a header file
#define LED_PIN     GPIO_PIN_4

//We need to pick the tick_per_clock as low as reasonably possible.
//But is also defines are upper time:
// CLK  Divider     Single      Max
// 24M  /1          41,66ns     2730ns
// 12M  /2          83,33ns     5416ns
//  8M  /3          125ns       8192ns

static TIM_HandleTypeDef TimHandle = {
    /* TIM16, initial frequency = 24,000,000
     * Prescale = 3 => 24 / 3 = 8,000,000 Mhz run rate
     * Period = 8000 => 8Mhz / 8000 = 1Khz
     * Then count to 8000 gives 1000 interrupts per second or 1Khz */
    .Instance = TIM16,
    .Init.Prescaler         = TIMER_DIVIDER - 1, // = 600
    .Init.Period            = 40000, //1Hz
    .Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1,
    .Init.CounterMode       = TIM_COUNTERMODE_UP,
    .Init.RepetitionCounter = 1 - 1,
    .Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE,
};

//TODO move to common header
#define ARRAY_SIZE(a)  (sizeof(a)/sizeof(a[0]))

#define FIFO_SIZE 16 //Must be a power of 2

_Static_assert(FIFO_SIZE > K, "FIFO_SIZE needs to be able to contain at least a full K of barks");
_Static_assert((FIFO_SIZE & (FIFO_SIZE - 1)) == 0 , "FIFO_SIZE needs to be a power of 2 to make access FAST");

/* TODO should we convert to use a size?
 * That is easier and might faster to check */
static struct {
    volatile int write; //points to empty spot
    volatile int read; //point to spot that should be read
    uint32_t buf[FIFO_SIZE];
} fifo = {.write = 1, .read = 0};

static bool fifo_is_empty(void)
{
    /* No need to check for % FIFO_SIZE.
     * We assume we do not write if fifo is full */
    return (fifo.read + 1) == fifo.write;
}

static bool fifo_is_full(void)
{
    return fifo.read %FIFO_SIZE == fifo.write %FIFO_SIZE;
}

static uint32_t fifo_read(void)
{
    return fifo.buf[++fifo.read % FIFO_SIZE];
}

/* Supports a single writer only! */
//TODO rename this function to bark_add or something
void fifo_write(bool bit, uint16_t tmo)
{
    printf("Adding tot %d %d @%d\r\n", bit, tmo, fifo.write);
    if (fifo_is_full()) {
        printf("Fifo full!\r\n");
        return; //Drop it, sorry. Programmer error
    }

    uint32_t d = tmo | bit << 31;

    fifo.buf[fifo.write % ARRAY_SIZE(fifo.buf)] = d;
    fifo.write++;
    /* If ISR not enabled the code is not running. Start it*/
    if (__HAL_TIM_GET_IT_SOURCE(&TimHandle, TIM_IT_UPDATE) == RESET) {
        /* Generate event to RESET ALL counter values.
         * This is need to get accurate timing for the first bit. */
        TIM16->EGR = TIM_EGR_UG;
        __HAL_TIM_ENABLE_IT(&TimHandle, TIM_DIER_UIE);
        //__HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE);
        printf("Enable ISR %ld %ld\r\n", TIM16->DIER, TIM16->CR1);
    }
}

static int cnt = 0;

void TIM16_IRQHandler(void)
{
    /* This code needs to FAAAAST.
     * But like REALLY fast, like cut corners AND circles to be fast.
     *
     * That means no crappy bloated HAL code here!
     * Just old skool register writing */
    cnt++;

    if(fifo_is_empty()) {
        //TODO Always off or on if no bit available? What is the 'rest' state?
        GPIOA->BRR = LED_PIN;
        /* Disable the interrupt, to force not getting here again and signal to the write we are done */
        TIM16->DIER = 0;
        return;
    }
    uint32_t t = fifo_read();
    uint32_t tmo = t & 0xFFFF; //TODO MEGA hack, just write all 32 bit to ARR. Only lower 16 bit are ever used. Saves us a bit manipulation
    uint32_t bit = !!(t & 1u<<31);

    /* Write output TODO findout if conditional shift + only writing BSRR is less ASM */
    if (bit) {
        /* BSRR => Bit Set Reset Register. Write 1 to set I/O */
        GPIOA->BSRR = LED_PIN;
    } else {
        /* BRR => Bit Reset Register. Write 1 to clear I/O */
        GPIOA->BRR = LED_PIN;
    }

    /* Update reload register with new timeout */
    TIM16->ARR = tmo;

    /* Clear our interrupt flag */
    TIM16->SR = ~TIM_IT_UPDATE;
}

void debug_print(void)
{
    printf("Cnt %d / %ld\r\n", cnt, TIM16->CNT);
}

void raddr_output_init(void)
{
    /* First enable the clocking towards TIM16 */
    __HAL_RCC_TIM16_CLK_ENABLE();

    /* Setup timer and start.
     * Note that we do NOT enable the timer internal interrupt just yet */
    HAL_TIM_Base_Init(&TimHandle);
    HAL_TIM_Base_Start_IT(&TimHandle);

    /* We need to be the highest priority, to ensure rock solid jitter free output! */
    HAL_NVIC_SetPriority(TIM16_IRQn, 0, 0);

    /* Enable our interrupt */
    HAL_NVIC_EnableIRQ(TIM16_IRQn);
}

