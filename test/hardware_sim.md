# Hardware Simulation

Unit tests run natively, not on the embedded target, so they don't have access to any real hardware peripherals. Instead, we need to simulate the data moving in and out of them so that unit tests can be run on arbitrary data.

These hardware simulations only run at the data link layer, so the exact timing and bit level framing is not accurate. For example, UART is simulated by reading and writing to a buffer, so the baud rate, parity, etc is not simulated.

## UART

Only supports 8 data bits, 1 stop bit, no parity. FIFO and IRQ are not simulated.

The UART simulation uses an internal buffer to store data being sent and received. Reads and writes to UART just return values from the array.

Most pico-sdk uart functions are just redefined as empty functions. The following functions are defined:

* `uart_is_readable` - returns 

To control the UART interface:

* `void uart_sim_init()`

## GPIO

TODO

## SPI

TODO
