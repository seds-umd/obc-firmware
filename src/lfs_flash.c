#include "flash.h"
#include "littlefs/lfs.h"
#include "pico/stdlib.h"


lfs_t lfs;
lfs_file_t file;

int flash_bd_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size) {
    uint32_t addr = block * c->block_size + off;
    flash_read_bytes(addr, (uint8_t*)buffer, size);
    return LFS_ERR_OK;
}

int flash_bd_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size) {
    uint32_t addr = block * c->block_size + off;
    const uint8_t *src = (const uint8_t*)buffer;
    size_t remaining = size;
    
    while (remaining > 0) {
        size_t chunk_size = remaining > 256 ? 256 : remaining;
        
        flash_write_bytes(addr, (uint8_t*)src, chunk_size);
        flash_wait_done(); 
        
        addr += chunk_size;
        src += chunk_size;
        remaining -= chunk_size;
    }
    
    return LFS_ERR_OK;
}

int flash_bd_erase(const struct lfs_config *c, lfs_block_t block) {
    uint32_t addr = block * c->block_size;
    flash_erase_4k(addr);
    flash_wait_done(); 
    
    return LFS_ERR_OK;
}

int flash_bd_sync(const struct lfs_config *c) {
    flash_wait_done();
    return LFS_ERR_OK;
}

struct lfs_config cfg = {
    
    .read = flash_bd_read,
    .prog = flash_bd_prog,
    .erase = flash_bd_erase,
    .sync = flash_bd_sync,

    
    .read_size = 16,
    .prog_size = 16,
    .block_size = 4096,
    .block_count = 128,
    .cache_size = 16,
    .lookahead_size = 16,
    .block_cycles = 500,
};

void littlefs_mount(){
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }
    lfs_mkdir(&lfs, "/telemetry");
}
