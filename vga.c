static lcdConfig_t lcd_config = {

    /* width & height */
    LCD_WIDTH, LCD_HEIGHT,       

    /* max width & max height */
    MAX_WIDTH, MAX_HEIGHT,
    
    /* video_on_line */
    VIDEO_ON_LINE,  

    /* pll_ctrl/clk_ctrl */
    0x665, 0x7c2,   //25.3MHz
    /* gamma_cntl_port */
    (1 << LCD_GAMMA_EN) | (1 << LCD_GAMMA_RVS_OUT) | (0 << LCD_GAMMA_VCOM_POL),
    /* gamma_vcom_hswitch_addr */
    0,        
    /* rgb_base_addr */
    0xf0,
    /* rgb_coeff_addr */
    0x74a,
    /* pol_cntl_addr */
    (0x1 << LCD_HS_POL) | (0x1 << LCD_VS_POL) | (0x1 << LCD_CPH1_POL) | (0x1 << LCD_CPH3_POL),      
    /* dith_cntl_addr */
    0,
    /* STH1 hs/he/vs/ve */
    19, 120, 0, MAX_HEIGHT-1,//63, 66, 0, MAX_HEIGHT - 1,
    /* STH2 hs/he/vs/ve */
    0, 0, 0, 0,//63, 66, 0, MAX_HEIGHT - 1,
    /* OEH hs/he/vs/ve */
    0, 0, 0, 0,//602, 590, 0, MAX_HEIGHT - 1,
    /* vcom hswitch_addr/vs/ve */
    0, 0, MAX_HEIGHT - 1,
    /* CPV1 hs/he/vs/ve */    //CKV
    0, 0, 0, 0,//574, 616, 0, MAX_HEIGHT-1,
    /* CPV2 hs/he/vs/ve */
    0, 0, 0, 0,
    /* STV1 hs/he/vs/ve */
    0, MAX_WIDTH - 1, 6, 4,//0, MAX_WIDTH-1, VIDEO_ON_LINE, VIDEO_ON_LINE,
    /* STV2 hs/he/vs/ve */
    0, 0, 0, 0,//0, MAX_WIDTH-1, VIDEO_ON_LINE, VIDEO_ON_LINE,
    /* OEV1 hs/he/vs/ve */     //OEV
    0, 0, 0, 0,//536, 590, 0, MAX_HEIGHT-1,
    /* OEV2 hs/he/vs/ve */
    0, 0, 0, 0,//0, 2000, 0, 2000,
    /* OEV3 hs/he/vs/ve */
    0, 0, 0, 0,
    /* inv_cnt_addr */
    (1 << LCD_INV_EN)|(12 << LCD_INV_CNT),
    /* tcon_misc_sel_addr */
    (1 << LCD_STV1_SEL)|(1 << LCD_STV2_SEL),
    /* dual_port_cntl_addr */
    (0x1 << LCD_TTL_SEL)|(0x1 << LCD_ANALOG_SEL_CPH3)|(0x1 << LCD_ANALOG_3PHI_CLK_SEL),
    LCD_ANALOG,
    4,
    3
};

static VGAConfig_t VGA_config = {

    /* width & height */
    640, 480,       

    /* max width & max height */
    800, 525,

    /* pll_ctrl/clk_ctrl,  */    //data clock: 25.2MHz, hsync: 31.5khz, vsyn:60Hz
    0x40f, 0x561,  
    
    /* hsync_start_point & hsync_end_point */
    95, 0,
    
    /* vsync_start_point & vsync_end_point */
    0, 0,
    
    /* vsync_start_line & vsync_end_line*/
    0, 2,
    
    /* de_start_point & de_end_point */ 
    143, 143+640-1,
    
    /* de_start_line & de_end_line */
    35, 35+480,
    
    /* Hsync_Left_Border   &  Hsync_Right_Border*/
//    8, 8,
    
    /* Vsync_Top_Border   &  Vsync_Bottom_Border */
    8, 8,     
    /* flags, bit0: control swap pb pr; 
       bit1: control Data input from VFIFO; 
       bit[7:4]: control gldelay_data, if "1" : gldelay_data = gldelay_progressive_480P, else gldelay_data = gldelay_progressive_VGA  */
    0,
       
   /*screen_width & screen_height*/
	4, 3
};

set_VGA_config(&VGA_config);