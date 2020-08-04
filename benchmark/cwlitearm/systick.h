#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <stdint.h>


void systick_setup(void);
uint64_t hal_get_time(void);

#endif
