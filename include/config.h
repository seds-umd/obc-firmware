#pragma once

#include "hardware/irq.h"
#include "hardware/spi.h"
#include "hardware/uart.h"

// WARNING: current pinouts are for debugging, must change for actual OBC
// hardware

////////// UART //////////

// OpenLST
#define OPENLST_UART_ID uart0
#define OPENLST_UART_IRQ UART0_IRQ
#define OPENLST_UART_BAUD 115200
#define OPENLST_UART_TX 12
#define OPENLST_UART_RX 13
#define OPENLST_UART_CTS 14
#define OPENLST_UART_RTS 15

// NO_FLOW_CONTROL can be specified when running cmake
#ifdef NO_FLOW_CONTROL
#define OPENLST_FLOW false
#else
#define OPENLST_FLOW true
#endif

// Buffer size must be a power of 2
#define OPENLST_RX_BUF_LEN 1024

// TX buffer count must be <=32 because status is stored as uint32_t bitmask
#define OPENLST_TX_BUF_COUNT 4

// 2s delay on power cycle before setting pin back to default
#define OPENLST_POWER_CYCLE_DELAY_US 2000 * 1000

// Packet format parameters, max_payload is not including start or length bytes
#define OPENLST_MAX_PAYLOAD 251
#define OPENLST_START_0 0x22
#define OPENLST_START_1 0x69

// PIB
#define PIB_UART_ID uart1
#define PIB_UART_BAUD 115200
#define PIB_UART_TX 4
#define PIB_UART_RX 5

////////// DATA FLASH //////////

#define DATA_FLASH_SPI spi0
#define DATA_FLASH_RX 0
#define DATA_FLASH_CS 1
#define DATA_FLASH_SCK 2
#define DATA_FLASH_TX 3

// TODO: figure out how to make faster
#define DATA_FLASH_BAUD 125000000 / 4

////////// GPIO //////////

#define ANTENNA_DEPLOY_PIN 16
#define OPENLST_PWR_PIN 17
#define DEBUG_PIN 19

////////// SETTINGS //////////

// Size of scheduler task table
// The only time this should ever be defined elsewhere is in unit tests
#ifndef SCHEDULER_MAX_TASKS
#define SCHEDULER_MAX_TASKS 20
#endif

// Watchdog timeout - longest command should be program flash erase at 2s
#define WATCHDOG_TIMEOUT_MS 8000

////////// UPDATER //////////
// See bl_common.h for flash layout settings

// Update chunk size
#define UPDATER_CHUNK_SIZE 128

// Written to watchdog scratch 0 to indicate an update was applied
#define UPDATER_REBOOT_MAGIC 0x5aede6a9

// Written to watchdog scratch 0 to indicate we want to stay in bootloader
#define BL_MAGIC 0xacb09b3c
