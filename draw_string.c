//dpf5
af_osd_draw_rect(OSD_LAYER_HW_LAYER0,100,100,160,50, 0, 1);
af_osd_draw_string(OSD_LAYER_HW_LAYER0, str,strlen(str),110,110,2, af_get_default_font_group_id());

//dpf2
AWClearRectTxt(0,0,800,600);
AWAsciiPrintf(FORMAT_MSG_X,FORMAT_MSG_Y,fomatting_str);