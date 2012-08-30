serial_puts("\nDLL test:");
	for (k = 0; k < ((ddr_setting->ddr_ctrl&(1<<7))?0x2:0x4); k++) {
        serial_puts("\nlane:");
		serial_put_hex(k,8);
		serial_puts(" ");
		for (i = 0; i < 7; i++) {

		for (m = 0; m < MAX_PATERN; m++) {
		for (n = 0; n < TEST_COUNT; n++) {
        ddr_setting->init_pctl(ddr_setting);

				if (!start_ddr_config()) {
					return 1;
				}

				APB_Wr(PCTL_DTUCFG_ADDR, (k << 10) | 0x01); // select byte lane, & enable DTU
				set_result(Tra);
				APB_Wr(PCTL_DLLCR0_ADDR+(k<<2), (APB_Rd(PCTL_DLLCR0_ADDR+(k<<2)) & ~(0xf<<14)) | (dllcr[i]<<14));

                APB_Wr(PCTL_DTUWD0_ADDR, phd[m][0]);
                APB_Wr(PCTL_DTUWD1_ADDR, phd[m][1]);
                APB_Wr(PCTL_DTUWD2_ADDR, phd[m][2]);
                APB_Wr(PCTL_DTUWD3_ADDR, phd[m][3]);
                APB_Wr(PCTL_DTUWACTL_ADDR, n*0x20);
                APB_Wr(PCTL_DTURACTL_ADDR, n*0x20);
                if (!end_ddr_config()) {
					return 1;
				}

				if(!dtu_enable())
                    return 3;

         //serial_puts("  ");
		//serial_put_hex(check_dtu(),8);
                if(check_dtu()<16){
                    serial_puts("x");
                    goto DLL_END;
                }
        }

        }
DLL_END1:
         serial_puts("o");
DLL_END:
         ;
        }
	}