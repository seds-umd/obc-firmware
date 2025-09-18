#pragma once

#include "external/littlefs/lfs.h"
#include "flash.h"

int flash_bd_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int flash_bd_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int flash_bd_erase(const struct lfs_config *c, lfs_block_t block);
int flash_bd_sync(const struct lfs_config *c);

void littlefs_mount();
