#ifndef WOLF_H
#define WOLF_H

#include <stdbool.h>
#include <stdint.h>

#define T0L 16
#define T0H 4
#define T1L 4
#define T1H 16
#define STFU (T0L + T0H)
// if it is quit for longer than 4 bits consider it a reset
#define TRES (4*(T0L + T0H))

#define K 1

// Start of transmission
// Will be followed by either a HOWL or a BARK
#define GROWL 1
// End of transmission
// Will be quiet for at least TRES
#define HOWL 1
// Will be followed by a dataframe of K bits
#define BARK (!HOWL)

enum states {
    S_REST,       //do nothing
    S_ALERT,      //listen for howl
    S_HOWL,       //transmit frame
    S_BARK        //copy frame
};

extern void gpio_set(int);
extern int gpio_get(void);
extern void sleep_ns(int);
extern bool K_BINARY_INPUTS[K];
extern void update_input(void);

static inline void bark(int bit)
{
    gpio_set(1);
    sleep_ns(bit?T1H:T0H);
    gpio_set(0);
    sleep_ns(bit?T1L:T0L); //wait low time of bit
}

static inline void bark_full(int bit)
{
    bark(bit);
    sleep_ns(STFU);
}

#endif