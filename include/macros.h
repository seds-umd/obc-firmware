#pragma once

// Get rid of unused parameter warnings
#define UNUSED(x) (void)(x)

////////// Simulation macros //////////
#ifdef __arm__

// UART data register
#define UART_DR(uart) (uint8_t)(uart_get_hw(uart)->dr)

#else // __arm__

#define __not_in_flash_func(func_name) func_name

#endif // __arm__
