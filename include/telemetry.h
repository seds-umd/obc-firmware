#pragma once

#include <stdint.h>

struct telem_struct {
    uint32_t uptime_ms; 
    uint16_t telem_age_ms; 
    uint16_t v_mb_batt_mv; 
    uint16_t v_mb_4v2_mv; 
    uint16_t v_mb_3v3_mv; 
    uint16_t v_lst_4v2_mv;
    uint16_t v_lst_3v3_mv;
    //OBC Temps in the schematic
    uint16_t v_gps0_mv;
    uint16_t v_gps1_mv;
    uint16_t v_gps2_mv;

    uint16_t i_mb_3v3_obc_ma; 
    uint16_t i_mb_3v3_gps_ma; 
    uint16_t i_mb_3v3_lst_ma;
    uint16_t i_mb_4v2_lst_ma;
    uint16_t i_mb_batt_ma; 
    uint16_t t_lst0_cc;
    uint16_t t_lst1_cc;
    uint16_t t_obc0_cc; 
    uint16_t t_obc1_cc; 
    uint16_t t_rp2040_cc;
    uint16_t t_cc1110_cc;
    uint32_t lst_uptime_ms;
    uint32_t lst_uart1_rx;
    int8_t lst_rssi_last;
    int8_t lst_rssi_cont;
    uint8_t lst_lqi_last;
    int8_t lst_freqest_last;
    uint32_t lst_packets_sent;
    uint32_t lst_cs_count;
    uint32_t lst_packets_good;
    uint32_t lst_reject_crc;
    uint32_t lst_reject_other;
    //uint32_t lst_freq;
    //uint16_t lst_fsctrl;
    //uint8_t lst_chan_bw;
    //uint8_t lst_drate_e;
    //uint8_t lst_drate_m;
    //uint8_t lst_deviatn;
    //uint8_t lst_power;
    uint16_t boot_count;
};

void telem_update();

struct telem_struct *telem_get();

void update_boot_count();

int read_boot_count();