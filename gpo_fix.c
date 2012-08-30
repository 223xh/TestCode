        char str_info[50];
        unsigned i = 10000;
        memset(str_info, 0, 50);
        
        AVTimeDly(8000);
        
        irtc_write(RTC_GPO_COUNTER_ADDR, 0x500000);
        //AVTimeDly(10000);
        AVTimeDly(20);
        if((*RTC_REG1) & (1<<2))
        {
            sprintf(str_info,"GPO state is error!Now, fixing...");
            af_osd_draw_rect(OSD_LAYER_HW_LAYER0,100,100,400,50, 0, 1);
            af_osd_draw_string(OSD_LAYER_HW_LAYER0, str_info,strlen(str_info),110,110,2, af_get_default_font_group_id());
            AVTimeDly(2000);
            
            while(--i)
            {
                static_reg_write(0x1e);
                static_reg_write(0xe);
            }
            AVTimeDly(20);
            static_reg_write(0x4);
            AVTimeDly(20);
            
            memset(str_info, 0, 50);
            if(!((*RTC_REG1) & (1<<2)))
            {
                sprintf(str_info,"GPO state is fixed.");
            }
            else
            {
                sprintf(str_info,"GPO state is fixed failure.");
            }
            af_osd_draw_rect(OSD_LAYER_HW_LAYER0,100,100,400,50, 0, 1);
            af_osd_draw_string(OSD_LAYER_HW_LAYER0, str_info,strlen(str_info),110,110,2, af_get_default_font_group_id());
            AVTimeDly(2000);
            
            irtc_write(RTC_GPO_COUNTER_ADDR, 0x100000);
            
            for(i=5;i>0;i--)
            {
                memset(str_info, 0, 50);
                sprintf(str_info,"System will reboot in %u seconds.",i);
                af_osd_draw_rect(OSD_LAYER_HW_LAYER0,100,100,400,50, 0, 1);
                af_osd_draw_string(OSD_LAYER_HW_LAYER0, str_info,strlen(str_info),110,110,2, af_get_default_font_group_id());
                AVTimeDly(1000);
            }
            
            set_wakeup_time(5);
            power_hold(0);
            AVTimeDly(1000);
        }
        else
        {
            sprintf(str_info,"GPO state is ok!");
            af_osd_draw_rect(OSD_LAYER_HW_LAYER0,100,100,400,50, 0, 1);
            af_osd_draw_string(OSD_LAYER_HW_LAYER0, str_info,strlen(str_info),110,110,2, af_get_default_font_group_id());
            irtc_write(RTC_GPO_COUNTER_ADDR, 0x100000);
        }