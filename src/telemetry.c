#include "telemetry.h"
#include "logging.h"
#include "lfs_flash.h"
#include "amux.h"
#include "pico/time.h"

#include "littlefs/lfs.h"

static struct telem_struct telem;

void telem_update() {
    //Set telem age
    telem.telem_age_ms = to_ms_since_boot(get_absolute_time()) - telem.uptime_ms;
    telem.uptime_ms = to_ms_since_boot(get_absolute_time());

    // Update analog telemetry
    telem.v_mb_batt_mv = read_and_convert(4);
    telem.v_mb_3v3_mv = read_and_convert(5);
    telem.v_mb_4v2_mv = read_and_convert(8);
    //Listed as OBC temperature on the schematics
    telem.v_gps0_mv = read_and_convert(13);
    telem.v_gps1_mv = read_and_convert(14);
    telem.v_gps2_mv = read_and_convert(15);

    telem.i_mb_3v3_gps_ma = read_and_convert(1);
    telem.i_mb_3v3_obc_ma = read_and_convert(0);
    telem.i_mb_batt_ma = read_and_convert(6);
    telem.i_mb_3v3_lst_ma = read_and_convert(9);
    telem.i_mb_4v2_lst_ma = read_and_convert(10);

    telem.t_obc0_cc = read_and_convert(3);
    telem.t_obc1_cc = read_and_convert(2);
    telem.t_lst0_cc = read_and_convert(11);
    telem.t_lst1_cc = read_and_convert(12);

    telem.t_rp2040_cc = readrp2040Temp();


    // Update OpenLST telemetry
    //Requires command and implementation in openlst firmaware 

    //Boot Count
    //Requires read to filesystem
    telem.boot_count = read_boot_count();
}

struct telem_struct *telem_get() {
    telem_update();
    return &telem;
}

void telem_log(){
    char path[16] = "/telem/boot_xxx";
    //Convert to char
    path[14] = telem.boot_count%10+'0';
    path[13] = (telem.boot_count%100)/10+'0';
    path[12] = (telem.boot_count%1000)/100+'0';
    
    //lfs_file_open(&lfs, &file, path, LFS_O_RDWR | LFS_O_CREAT);
    //lfs_file_write(&lfs, &file, telem, sizeof(telem));
    //lfs_file_close(&lfs, &file);
}


void update_boot_count(){
    uint16_t boot_count = 0;
    //lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    //lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    boot_count += 1;
    //lfs_file_rewind(&lfs, &file);
    //lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));
    //lfs_file_close(&lfs, &file);
}

int read_boot_count(){
    uint16_t boot_count = 0;
    //lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    //lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));
    //lfs_file_close(&lfs, &file);
    return boot_count;
}
    