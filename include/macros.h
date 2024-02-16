// Get rid of unused parameter warnings
#define UNUSED(x) (void)(x)

////////// Simulation macros //////////
#ifdef __arm__

// UART data register
#define UART_DR(uart) (uint8_t)(uart_get_hw(uart)->dr)

#endif // __arm__
