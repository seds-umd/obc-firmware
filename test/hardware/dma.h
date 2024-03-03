// Adapted from hardware/dma.h

/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stdint.h>

#include "macros.h"

#define DREQ_UART0_TX 0
#define DREQ_UART1_TX 0
#define DMA_IRQ_0 0

typedef struct {
    uint32_t ctrl;
} dma_channel_config;

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

static inline void channel_config_set_dreq(dma_channel_config *c, unsigned int dreq) {
    UNUSED(c);
    UNUSED(dreq);
}

static inline void dma_channel_configure(unsigned int channel,
                        const dma_channel_config *config,
                        volatile void *write_addr,
                        const volatile void *read_addr,
                        unsigned int transfer_count,
                        bool trigger) {
    UNUSED(channel);
    UNUSED(config);
    UNUSED(write_addr);
    UNUSED(read_addr);
    UNUSED(transfer_count);
    UNUSED(trigger);
}

static inline void dma_channel_set_irq0_enabled(unsigned int channel, bool enabled) {
    UNUSED(channel);
    UNUSED(enabled);
}
