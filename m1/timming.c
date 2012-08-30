#if DDR_W972GG6JB==CONFIG_DDR_TYPE
STATIC_PREFIX_DATA struct ddr_set __ddr_setting __attribute__((section(".setting")))
={
                    .cl             =   6,
                    .t_faw          =  20,//14,//1k page //18, 2k page  //20,
                    .t_mrd          =   2,
                    .t_1us_pck      = 400,
                    .t_100ns_pck    =  40,
                    .t_init_us      = 511,
                    .t_ras          =  18,
                    .t_rc           =  22
                    .t_rcd          =   6,
                    .t_refi_100ns   =  78,
                    .t_rfc          =  80,//51,//1G //80, 2G
                    .t_rp           =   6,
                    .t_rrd          =   4,//3,//x4x8 //4, x16
                    .t_rtp          =   3,
                    .t_wr           =   6,
                    .t_wtr          =   4,
                    .t_xp           =   2,
                    .t_xsrd         =   0,   // init to 0 so that if only one of them is defined, this is chosen
                    .t_xsnr         =   0,
                    .t_exsr         = 200,
                    .t_al           =   0,   // Additive Latency
                    .t_clr          =   6,   // cas_latency for DDR2 (nclk cycles)
                    .t_dqs          =   2,   // distance between data phases to different ranks
                    .pad =   0,
                    .ddr_ctrl       =   DDR_CTRL,
    .ddr_pll_cntl   =   0x110221,//400
}
;
#endif


STATIC_PREFIX_DATA struct ddr_set __ddr_setting __attribute__((section(".setting")))
={
                    .cl             =   6,
                    .t_faw          =  14,
                    .t_mrd          =   2,
                    .t_1us_pck      = 400,
                    .t_100ns_pck    =  40,
                    .t_init_us      = 511,
                    .t_ras          =  18,
                    .t_rc           =  23,
                    .t_rcd          =   6,
                    .t_refi_100ns   =  78,
                    .t_rfc          =  51,
                    .t_rp           =   6,
                    .t_rrd          =   4,
                    .t_rtp          =   3,
                    .t_wr           =   6,
                    .t_wtr          =   3,
                    .t_xp           =   2,
                    .t_xsrd         =   0,   // init to 0 so that if only one of them is defined, this is chosen
                    .t_xsnr         =   0,
                    .t_exsr         = 200,
                    .t_al           =   0,   // Additive Latency
                    .t_clr          =   6,   // cas_latency for DDR2 (nclk cycles)
                    .t_dqs          =   2,   // distance between data phases to different ranks
                    .pad			=   0,
                    .ddr_ctrl       =   DDR_CTRL,
				    .ddr_pll_cntl   =   0x11021c,//0x110664,//400
}
