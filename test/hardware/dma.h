// Adapted from hardware/dma.h

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "macros.h"
#include "misc.h"

#include <stdbool.h>
#include <stdint.h>

#define DREQ_UART0_TX 0
#define DREQ_UART1_TX 0
#define DMA_IRQ_0 0

typedef struct {
    uint32_t ctrl;
} dma_channel_config;

typedef struct {
    volatile uint32_t ints0;
} dma_hw_t;

extern dma_hw_t *dma_hw;

enum dma_channel_transfer_size {
    DMA_SIZE_8 = 0,    ///< Byte transfer (8 bits)
    DMA_SIZE_16 = 1,   ///< Half word transfer (16 bits)
    DMA_SIZE_32 = 2    ///< Word transfer (32 bits)
};

static inline int dma_claim_unused_channel(bool required) {
    UNUSED(required);

    return 0;
}

static inline dma_channel_config dma_channel_get_default_config(int channel) {
    UNUSED(channel);

    dma_channel_config c = {0};

    return c;
}

static inline void channel_config_set_transfer_data_size(dma_channel_config *c, enum dma_channel_transfer_size size) {
    UNUSED(c);
    UNUSED(size);
}

static inline void channel_config_set_write_increment(dma_channel_config *c, bool incr) {
    UNUSED(c);
    UNUSED(incr);
}

static inline void channel_config_set_read_increment(dma_channel_config *c, bool incr) {
    UNUSED(c);
    UNUSED(incr);
}

static inline void channel_config_set_dreq(dma_channel_config *c, uint dreq) {
    UNUSED(c);
    UNUSED(dreq);
}

static inline void dma_channel_configure(uint channel,
                        const dma_channel_config *config,
                        volatile void *write_addr,
                        const volatile void *read_addr,
                        uint transfer_count,
                        bool trigger) {
    UNUSED(channel);
    UNUSED(config);
    UNUSED(write_addr);
    UNUSED(read_addr);
    UNUSED(transfer_count);
    UNUSED(trigger);
}

static inline void dma_channel_set_irq0_enabled(uint channel, bool enabled) {
    UNUSED(channel);
    UNUSED(enabled);
}

static inline void dma_channel_set_trans_count(uint channel, uint32_t trans_count, bool trigger) {
    UNUSED(channel);
    UNUSED(trans_count);
    UNUSED(trigger);
}

static inline void dma_channel_set_read_addr(uint channel, const volatile void *read_addr, bool trigger) {
    UNUSED(channel);
    UNUSED(read_addr);
    UNUSED(trigger);
}

static inline bool dma_channel_is_busy(uint channel) {
    UNUSED(channel);

    return false;
}
