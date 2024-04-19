#include "bl_common.h"

#include "hardware/flash.h"
#include "RP2040.h"

#include <stdint.h>

int main() {
    // https://github.com/usedbytes/rp2040-serial-bootloader/blob/f6f0d61fc03447a1401d3f9d158f35331bf0eb98/main.c#L80
    uint32_t vtor = XIP_BASE + BL_APP_START;
    uint32_t reset_vector = *(volatile uint32_t *)(vtor + 0x04);

    SCB->VTOR = (volatile uint32_t)(vtor);

    asm volatile("msr msp, %0" ::"g"(*(volatile uint32_t *)vtor));
    asm volatile("bx %0" ::"r"(reset_vector));
}
