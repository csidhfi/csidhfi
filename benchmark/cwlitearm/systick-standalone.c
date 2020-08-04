/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Thomas Otto <tommi@viadmin.org>
 * Copyright (C) 2012 Benjamin Vernoux <titanmkd@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "systick.h"
#include <stdint.h>

#define MMIO32(addr) (*(volatile uint32_t *)(addr))
#define PPBI_BASE (0xE0000000U)
#define SCS_BASE (PPBI_BASE + 0xE000)
#define SYS_TICK_BASE (SCS_BASE + 0x0010)
#define STK_CSR MMIO32(SYS_TICK_BASE + 0x00)
#define STK_RVR MMIO32(SYS_TICK_BASE + 0x04)
#define STK_CVR MMIO32(SYS_TICK_BASE + 0x08)
#define STK_CSR_CLKSOURCE_LSB 2
#define STK_CSR_CLKSOURCE (1 << STK_CSR_CLKSOURCE_LSB)
#define STK_CSR_CLKSOURCE_AHB (1 << STK_CSR_CLKSOURCE_LSB)

#define STK_RVR_RELOAD 0x00FFFFFF
#define STK_CSR_TICKINT (1 << 1)
#define STK_CSR_ENABLE  (1 << 0)
#define STK_CVR_CURRENT 0x00FFFFFF

static void systick_set_clocksource(uint8_t clocksource)
{
  STK_CSR = (STK_CSR & ~STK_CSR_CLKSOURCE) | (clocksource & STK_CSR_CLKSOURCE);
}

static void systick_set_reload(uint32_t value)
{
  STK_RVR = (value & STK_RVR_RELOAD);
}

static void systick_interrupt_enable(void)
{
  STK_CSR |= STK_CSR_TICKINT;
}
static void systick_counter_enable(void)
{
  STK_CSR |= STK_CSR_ENABLE;
}

static uint32_t systick_get_value(void)
{
  return STK_CVR & STK_CVR_CURRENT;
}

void systick_setup(void)
{
  systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
  systick_set_reload(16777215);
  systick_interrupt_enable();
  systick_counter_enable();
}

static volatile unsigned long long overflowcnt = 0;
void SysTick_Handler(void)
{
  ++overflowcnt;
}

uint64_t hal_get_time(void)
{
  while (1) {
    unsigned long long before = overflowcnt;
    unsigned long long result = (before + 1) * 16777216llu - systick_get_value();
    if (overflowcnt == before) {
      return result;
    }
  }
}
