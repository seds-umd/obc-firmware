from typing import TypedDict

from openlst_tools.utils import unpack_cint

class Telemetry(TypedDict):
    uptime_ms: int
    telem_age_ms: int
    v_mb_batt_mv: int
    v_mb_4v2_mv: int
    v_mb_3v3_mv: int
    v_lst_4v2_mv: int
    v_lst_3v3_mv: int
    i_mb_3v3_obc_ma: int
    i_mb_3v3_gps_ma: int
    i_mb_3v3_lst_ma: int
    i_mb_4v2_lst_ma: int
    i_mb_batt_ma: int
    t_lst0_cc: int
    t_lst1_cc: int
    t_obc0_cc: int
    t_obc1_cc: int
    t_rp2040_cc: int
    t_cc1110_cc: int
    lst_uptime_ms: int
    lst_uart1_rx: int
    lst_rssi_last: int
    lst_rssi_cont: int
    lst_lqi_last: int
    lst_freqest_last: int
    lst_packets_sent: int
    lst_cs_count: int
    lst_packets_good: int
    lst_reject_crc: int
    lst_reject_other: int
    lst_freq: int
    lst_fsctrl: int
    lst_chan_bw: int
    lst_drate_e: int
    lst_drate_m: int
    lst_deviatn: int
    lst_power: int

    def decode(msg: bytes):
        msg = bytearray(msg)

        telem = Telemetry()

        def pop(n):
            return bytes([msg.pop(0) for _ in range(n)])

        telem['uptime_ms'] = unpack_cint(pop(4), 4, False)
        telem['telem_age_ms'] = unpack_cint(pop(2), 2, False)
        telem['v_mb_batt_mv'] = unpack_cint(pop(2), 2, False)
        telem['v_mb_4v2_mv'] = unpack_cint(pop(2), 2, False)
        telem['v_mb_3v3_mv'] = unpack_cint(pop(2), 2, False)
        telem['v_lst_4v2_mv'] = unpack_cint(pop(2), 2, False)
        telem['v_lst_3v3_mv'] = unpack_cint(pop(2), 2, False)
        telem['i_mb_3v3_obc_ma'] = unpack_cint(pop(2), 2, False)
        telem['i_mb_3v3_gps_ma'] = unpack_cint(pop(2), 2, False)
        telem['i_mb_3v3_lst_ma'] = unpack_cint(pop(2), 2, False)
        telem['i_mb_4v2_lst_ma'] = unpack_cint(pop(2), 2, False)
        telem['i_mb_batt_ma'] = unpack_cint(pop(2), 2, False)
        telem['t_lst0_cc'] = unpack_cint(pop(2), 2, False)
        telem['t_lst1_cc'] = unpack_cint(pop(2), 2, False)
        telem['t_obc0_cc'] = unpack_cint(pop(2), 2, False)
        telem['t_obc1_cc'] = unpack_cint(pop(2), 2, False)
        telem['t_rp2040_cc'] = unpack_cint(pop(2), 2, False)
        telem['t_cc1110_cc'] = unpack_cint(pop(2), 2, False)
        telem['lst_uptime_ms'] = unpack_cint(pop(4), 4, False)
        telem['lst_uart1_rx'] = unpack_cint(pop(4), 4, False)
        telem['lst_rssi_last'] = unpack_cint(pop(1), 1, True)
        telem['lst_rssi_cont'] = unpack_cint(pop(1), 1, True)
        telem['lst_lqi_lst'] = unpack_cint(pop(1), 1, False)
        telem['lst_freqest_last'] = unpack_cint(pop(1), 1, True)
        telem['lst_packets_sent'] = unpack_cint(pop(4), 4, False)
        telem['lst_cs_count'] = unpack_cint(pop(4), 4, False)
        telem['lst_reject_crc'] = unpack_cint(pop(4), 4, False)
        telem['lst_reject_other'] = unpack_cint(pop(4), 4, False)
        telem['lst_freq'] = unpack_cint(pop(4), 4, False)
        telem['lst_fsctrl'] = unpack_cint(pop(2), 2, False)
        telem['lst_chan_bw'] = unpack_cint(pop(1), 1, False)
        telem['lst_drate_e'] = unpack_cint(pop(1), 1, False)
        telem['lst_drate_m'] = unpack_cint(pop(1), 1, False)
        telem['lst_deviatn'] = unpack_cint(pop(1), 1, False)
        telem['lst_power'] = unpack_cint(pop(1), 1, False)

        # TODO: decode everything

        return telem
