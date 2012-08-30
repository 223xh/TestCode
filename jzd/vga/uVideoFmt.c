/*****************************************************************
**                                                              **
**  Copyright (C) 2007 Amlogic,Inc.                             **
**  All rights reserved                                         **
**                                                              **
**                                                              **
*****************************************************************/

#include <includes.h>		
#include <af_engine.h>		
#include <af_message.h>		
#include <af_msgtype.h>		
#include <af_global.h>		
#include <chipsupport.h>
#include <af_input.h>
#include "Drivers/hdmi/hdmi_process.h"
#include "Drivers/hdmi/hdmi.h"
#include "uVideoFmt_cnt.h"
#include <policy.h>

//#define EDGE_ADJUST 

#ifdef AML_APOLLO
#define VENC_VDAC_SETTING            0x1b7e
#define MPEG_BASE_ADDR               0xC1100000   
#define WRITE_MPEG_REG(reg, val) \
    *(volatile unsigned *)(MPEG_BASE_ADDR + ((reg)<<2)) = (val)
 
#endif
#include "bsp.h" 

#ifdef AML_APOLLO
static unsigned char max_osd_area_per_layer[MAX_OSD_LAYER_SUPPORTED] = {1, 16};
#else
static unsigned char max_osd_area_per_layer[MAX_OSD_LAYER_SUPPORTED] = {1, 1};
#endif

static int dbg_level = 0;


typedef struct{
    control_t* cntl;
    /* Add more members here */
    int backup_h;
    int backup_h_show;
    int backup_W4_reg;
    /*end*/
}uvideofmt_t;

/* access Handle/Array prop by these Macros*/
#define this_curVideoFmt                   af_get_prop_val(uvideofmt->cntl,PROP_CURVIDEOFMT_POS)
#define this_curVideoFmt_Set(val)          af_set_prop_val(uvideofmt->cntl,PROP_CURVIDEOFMT_POS,val)
#define this_outputmode                   af_get_prop_val(uvideofmt->cntl,PROP_OUTPUTMODE_POS)
#define this_outputmode_Set(val)          af_set_prop_val(uvideofmt->cntl,PROP_OUTPUTMODE_POS,val)
/*Macros defines end*/


static void uvideofmt_release(control_t* cntl)
{
    uvideofmt_t* uvideofmt = (uvideofmt_t *)(cntl->private_data);
    if(uvideofmt)
    {
        AVMem_free(uvideofmt);
    }

}

static uvideofmt_t* uvideofmt_init(control_t* cntl)
{
    int i=0;
    cond_item_t* temp_ptr=NULL;
    uvideofmt_t* uvideofmt= AVMem_calloc(sizeof(uvideofmt_t),1);
    if(uvideofmt)
    {
        uvideofmt->cntl=cntl;
    }
    return uvideofmt;
}


/* Handle Prop' method functions, #define HANDLE_METHOD_FUN to enable below code, remove functions that never been used */
//#define HANDLE_METHOD_FUN
#ifdef HANDLE_METHOD_FUN
#endif /*HANDLE_METHOD_FUN*/


/* message send functions, #define MSG_SEND_FUN to enable below code, remove functions that never been used */
//#define MSG_SEND_FUN
#ifdef MSG_SEND_FUN

#endif /*MSG_SEND_FUN*/


/* Above code is created by AfControlWizard */

/* Add User Code here */

/*end*/

/*****************************************************************
**                                                              **
**    Method  functions                                         **
**                                                              **
**                                                              **
*****************************************************************/
static int uVideoFmt_Init(control_t* cntl, cond_item_t* param)
{
    int ret=-1;
    uvideofmt_t* uvideofmt= uvideofmt_init(cntl);
    if(uvideofmt)
    {
        ret=0;
        /*  Add Initialize Code here , set ret to be -1 if fail*/
        this_curVideoFmt_Set(-1);
        /* end */
        cntl->private_data=uvideofmt;
    }
    if(ret<0)
        uvideofmt_release(cntl);
        

    
    
    return ret;
}


static int uVideoFmt_UnInit(control_t* cntl, cond_item_t* param)
{
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    /* Add UnInitialize code here */

    /* end */
    uvideofmt_release(cntl);
    return 0;
}

static int uVideoFmt_MsgProcess(control_t* cntl, cond_item_t* param)
{
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    message_t* msg = (message_t *)param;
    switch(msg->id){
        default:
            break;
    }
    return 0;
}

static int set_display_edge_adjust(int top_edge,int left_edge,int bottom_edge,int right_edge)
{
    video_display_edge_adjust(DISPLAY_ADJUST_KEEP_RATIO, top_edge, left_edge, bottom_edge, right_edge);
    video_display_edge_adjust(DISPLAY_ADJUST_STRETCH, top_edge, left_edge, bottom_edge, right_edge);
    video_display_edge_adjust(DISPLAY_4_3_TO_16_9_SUBTITLE, top_edge, left_edge, bottom_edge, right_edge);
    video_display_edge_adjust(DISPLAY_4_3_TO_16_9_NO_SUBTITLE, top_edge, left_edge, bottom_edge, right_edge);
    video_display_edge_adjust(DISPLAY_16_9_TO_4_3_TV, top_edge, left_edge, bottom_edge, right_edge);
}

static void set_vdac_0_1_2_output_type(int dac0, int dac1, int dac2)
{
    vdac_set_output_type(dac0, 0);
    vdac_set_output_type(dac1, 1);
    vdac_set_output_type(dac2, 2);
    if ((dac0!=VIDEO_SIGNAL_TYPE_CVBS)&&(dac1!=VIDEO_SIGNAL_TYPE_CVBS)&&(dac2!=VIDEO_SIGNAL_TYPE_CVBS)){
        vdac_set_output_type(VIDEO_SIGNAL_TYPE_SVIDEO_LUMA, 3);
        vdac_set_output_type(VIDEO_SIGNAL_TYPE_SVIDEO_CHROMA, 3); 
        vdac_set_output_type(VIDEO_SIGNAL_TYPE_CVBS, 3);
    }        
}

static int process_480I_CVBS(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_480I;
    video_mode->hdmode  = DISPCTL_MODE_480I;
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_SVIDEO_CHROMA, VIDEO_SIGNAL_TYPE_CVBS, VIDEO_SIGNAL_TYPE_SVIDEO_LUMA);
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
#ifdef AML_APOLLO              
    set_vdac0_1_2_output_type(VIDEO_SIGNAL_TYPE_SVIDEO_CHROMA, VIDEO_SIGNAL_TYPE_CVBS, VIDEO_SIGNAL_TYPE_SVIDEO_LUMA);            
#endif
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 720, 1200, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(7,9,7,9); 
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(0);
#endif 
    return 1;    
}

static int process_576I_CVBS(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_576I;
    video_mode->hdmode  = DISPCTL_MODE_576I;
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_SVIDEO_CHROMA, VIDEO_SIGNAL_TYPE_CVBS, VIDEO_SIGNAL_TYPE_SVIDEO_LUMA);
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
#ifdef AML_APOLLO              
    set_vdac0_1_2_output_type(VIDEO_SIGNAL_TYPE_SVIDEO_CHROMA, VIDEO_SIGNAL_TYPE_CVBS, VIDEO_SIGNAL_TYPE_SVIDEO_LUMA);            
#endif
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 720, 1200, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(8,10,8,10); 
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(0);
	//return 1;
#endif 
    return 3;    
}

static int process_480I(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_480I;
    video_mode->hdmode  = DISPCTL_MODE_480I;
    
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
    #ifdef AML_APOLLO      		    
    set_vdac0_1_2_output_type(VIDEO_SIGNAL_TYPE_INTERLACE_PR, VIDEO_SIGNAL_TYPE_SVIDEO_LUMA, VIDEO_SIGNAL_TYPE_INTERLACE_PB);
    #else
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_INTERLACE_PR, VIDEO_SIGNAL_TYPE_INTERLACE_Y, VIDEO_SIGNAL_TYPE_INTERLACE_PB);
    #endif
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 720,1200, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(7,9,7,9); 	
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(0);
#endif 
    return 1;
}

static int process_576I(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_576I;
    video_mode->hdmode  = DISPCTL_MODE_576I;
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
    #ifdef AML_APOLLO    		    
    set_vdac0_1_2_output_type(VIDEO_SIGNAL_TYPE_INTERLACE_PR, VIDEO_SIGNAL_TYPE_SVIDEO_LUMA, VIDEO_SIGNAL_TYPE_INTERLACE_PB);
    #else
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_INTERLACE_PR, VIDEO_SIGNAL_TYPE_INTERLACE_Y, VIDEO_SIGNAL_TYPE_INTERLACE_PB);
    #endif             
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 720, 1200, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(8,10,8,10);  
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(0);
	return 1;
#endif 
    return 3;
}

static int process_480P(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_480P;
    video_mode->hdmode  = DISPCTL_MODE_480P;
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_PROGRESSIVE_PR, VIDEO_SIGNAL_TYPE_PROGRESSIVE_Y, VIDEO_SIGNAL_TYPE_PROGRESSIVE_PB); 
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 720, 1200, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(7,9,7,9);
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(0);
#endif 
    return 1;    
}

static int process_576P(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_576P;
    video_mode->hdmode  = DISPCTL_MODE_576P;
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_PROGRESSIVE_PR, VIDEO_SIGNAL_TYPE_PROGRESSIVE_Y, VIDEO_SIGNAL_TYPE_PROGRESSIVE_PB); 
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 720, 1200, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(8,10,8,10);
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(0);
	return 1;
#endif 
    return 3;
}

static int process_720P(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_720P;
    video_mode->hdmode  = DISPCTL_MODE_720P;
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_PROGRESSIVE_PR, VIDEO_SIGNAL_TYPE_PROGRESSIVE_Y, VIDEO_SIGNAL_TYPE_PROGRESSIVE_PB); 
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);           
    //#ifdef APOLLO_PMP_TVOUT
    //af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 1280, 1200, MEMMAP_DEFAULT);
    //#else
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 1280, 736+160, MEMMAP_DEFAULT);
    //#endif 
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(11,19,11,19); 
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(0);
#endif 
    return 0;    
}
static int process_1080I(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_1080I;
    video_mode->hdmode  = DISPCTL_MODE_1080I;
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_PROGRESSIVE_PR, VIDEO_SIGNAL_TYPE_PROGRESSIVE_Y, VIDEO_SIGNAL_TYPE_PROGRESSIVE_PB); 
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
#ifdef FULL_1080OSD    
  	af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 1920, 1088+256+32, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#else   
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 960, 544+128+32, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 1, 1);
#endif
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(16,29,16,29); 
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(1);
#endif      
    return 2;
}
static int process_1080P(video_appmode_t *video_mode, int video_fd)
{
    video_mode->outmode = DISPCTL_MODE_1080P;
    video_mode->hdmode  = DISPCTL_MODE_1080P;
    set_vdac_0_1_2_output_type(VIDEO_SIGNAL_TYPE_PROGRESSIVE_PR, VIDEO_SIGNAL_TYPE_PROGRESSIVE_Y, VIDEO_SIGNAL_TYPE_PROGRESSIVE_PB); 
    ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
#ifdef FULL_1080OSD    
  	af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 1920, 1088+256+32, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
#else
	af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 960, 544+128+32, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 1, 1);
#endif
#ifdef	EDGE_ADJUST
	set_display_edge_adjust(27,29,27,29); 
#endif
#ifdef APOLLO_PMP_TVOUT
    change_ddr_burst_control(1);
#endif      
    return 2;
}

typedef struct
{
    int (*func)(video_appmode_t*, int);
}_VIDEO_SWTICH_PROCESS_;

static _VIDEO_SWTICH_PROCESS_ analog_switch_process [] =
{
    process_480I_CVBS,
    process_576I_CVBS,
    process_480I,
    process_576I,
    process_480P,
    process_576P,
    process_720P,
    process_1080I,
    process_1080P,
};

static _VIDEO_SWTICH_PROCESS_ hdmi_switch_process[] =
{
    process_480I_CVBS,
    process_576I_CVBS,
    process_480I,
    process_576I,
    process_480P,
    process_576P,
    process_720P,
    process_1080I,
    process_1080P,
};
   
static int set_analog_output(int video_format, video_appmode_t *video_mode)
{
    int resolution;
    int video_fd;
    video_fd=open("/dev/video",O_RDONLY);
    
#if (defined APOLLO_PMP_LCD)||(defined NIKE_PMP_LCD)
    power_off_backlight();
#endif
    resolution = analog_switch_process[video_format].func(video_mode, video_fd);
#ifdef APOLLO_PMP_TVOUT
    power_on_vdac();
#endif
    close(video_fd);
    return resolution;  
}

static int set_hdmi_output(int video_format, video_appmode_t *video_mode)
{
    int resolution;
    int video_fd;
    video_fd=open("/dev/video",O_RDONLY);
#if (defined APOLLO_PMP_LCD)||(defined NIKE_PMP_LCD)
    power_off_backlight();
#endif
    resolution = hdmi_switch_process[video_format].func(video_mode, video_fd);
#ifdef APOLLO_PMP_TVOUT
	power_off_vdac();
#endif
    close(video_fd);
    return resolution;   
}

#if (defined APOLLO_PMP_LCD)||(defined NIKE_PMP_LCD)
static int set_lcd_output(int video_format, video_appmode_t *video_mode)
{
    int resolution;
    int video_fd;
    video_fd=open("/dev/video",O_RDONLY);
    power_off_vdac();
    
    
    video_mode->outmode = DISPCTL_MODE_480P;
    video_mode->hdmode  = DISPCTL_MODE_LCD;
    if(bsp_reset_flag)
    {
        ioctl(video_fd,VIDEOIO_SETAPPMODE,video_mode);
    }
    ioctl(video_fd, VIDEOIO_SET_WIDEOPTION, VIDEO_WIDEOPTION_WIDE);
    
#ifdef APOLLO_PMP_LCD
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 800, 800, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
    resolution = 0;
#else
    af_osd_layer_create(OSD_LAYER_HW_LAYER0, OSD_LAYER_HW_LAYER0, 320, 800, MEMMAP_DEFAULT);
    af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
    resolution = 0;
#endif
    if(bsp_reset_flag)
        power_on_backlight();

#ifdef APOLLO_PMP_LCD
    change_ddr_burst_control(0);
#endif
    close(video_fd);
    return resolution; 
}
#endif

static int uVideoFmt_SetVideoType(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    int fmt = (int)(param[0]);
    int outputmode = (int)(param[1]);
    /* Add Method code here, Set ret to be -1 if fail */
    int displayctl_fd, resolution_data;
    video_appmode_t video_appmode = 
    {
        DISPCTL_MODE_1080P, 
        0, 
        0, 
        0, 
        DISPCTL_MODE_1080P
    };
    page_t *page = af_find_page(SYSTEM_DEFAULT_PAGE);

    if (this_curVideoFmt!=fmt || this_outputmode!= outputmode)
    {
            /* when NTSC/PAL CVBS output is enabled in first 3 video dacs,
             * enable pedestal. Component 480i has pedestal disabled.
             */   
#ifdef AML_NIKE
        fmt = 6;
#endif
#ifndef EDGE_ADJUST
		set_display_edge_adjust(0,0,0,0); 	
#endif
        af_osd_layer_destroy(OSD_LAYER_HW_LAYER0);
        
       
        switch(outputmode)
        {
            case 0://Analog
                clear_mio_mux(5, (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<7));
                //disable RGB data
                //clear_mio_mux(4, (1<<20) | (1<<22) | (1<<24));//6bits
                clear_mio_mux(4, (0x3f<<20));//8bits    
#ifdef APOLLO_PMP_TVOUT                            
                clear_mio_mux(4, 0x7c000000);       //disable hdmi output , 
#endif                                           	
                resolution_data = set_analog_output(fmt, &video_appmode);
                break;
                
            case 2://HDMI            	
                clear_mio_mux(5, (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<7));
                //disable RGB data
                //clear_mio_mux(4, (1<<20) | (1<<22) | (1<<24));//6bits
                clear_mio_mux(4, (0x3f<<20));//8bits
                resolution_data = set_hdmi_output(fmt, &video_appmode);      
                set_mio_mux(4, 0x7c000000);       //mux GPIOC_28 to hsync, mux GPIOC_29 to vsync, mux GPIOC_30 to DE, mux GPIOC_31 to pixel clock, mux GPIOC_32~GPIOC_55 to HDMI data lines,  
                Codec_PowerDown_Entile_Codec(0);    
                break;

#if (defined APOLLO_PMP_LCD) || (defined NIKE_PMP_LCD)
            case 1://Panel 
				#ifdef EDGE_ADJUST
            	set_display_edge_adjust(0,0,0,0); 	
				#endif
                clear_mio_mux(4, 0x7c000000);       //disable hdmi output , 
                //enable Tcon
                //set_mio_mux(5, (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<7) | (1<<8) | (1<<9));
                set_mio_mux(5, (1<<0) | (1<<2) | (1<<3) | (1<<4) | (1<<7));  
                //enable RGB data
                //set_mio_mux(4, (1<<20) | (1<<22) | (1<<24));//6bits
                set_mio_mux(4, (0x3f<<20));//8bits            	                       	
                resolution_data = set_lcd_output(fmt, &video_appmode);
                power_off_USBb();
                break;
#endif
            default:
                AVOS_printf("Please select the valid output mode!\n");
                break;
        }
        

        GF_SetHdmode(cntl, &video_appmode.hdmode);
    	af_set_current_resolution(resolution_data);
        af_init_osd(page);                	
    }
    this_curVideoFmt_Set(fmt);
    if(outputmode<=2&&outputmode>=0)
    {
	    this_outputmode_Set(outputmode);
	}
	
	 ///////////////////////////////////////////
		int video_fd;
		video_fd=open("/dev/video",O_RDONLY);
		//video_appmode_t video_appmode = {DISPCTL_MODE_480P, VIDEO_APPVMODE_480_480, VIDEO_APPHMODE_720, 0, DISPCTL_MODE_VGA};
		video_appmode_t video_appmode2 = {DISPCTL_MODE_480P, VIDEO_APPVMODE_480_480, VIDEO_APPHMODE_720, 0, DISPCTL_MODE_VGA};
		ioctl(video_fd, VIDEOIO_SETAPPMODE, &video_appmode2);
		WRITE_MPEG_REG(VENC_VDAC_SETTING,0);
    power_on_vdac();
 ///////////////////////////////////////////
       
    AVTimeDly(100);
   
    /* end */
    return ret;
}

void SetTVOutput(int curVideoFmt, int display_mode)
{
	TvOutputConfig_t *tv_output_config;
	int hdmode;

	switch(curVideoFmt){
		case 0: // NTSC-cvbs     
			hdmode = DISPCTL_MODE_480I;
			break;		
		case 1: // PAL-cvbs
			hdmode = DISPCTL_MODE_576I;
			break;
		case 2: // 480I
			hdmode = DISPCTL_MODE_480I;
			break;
		case 3: // 576I
			hdmode = DISPCTL_MODE_576I;
			break;
		case 4: // 480P
			hdmode = DISPCTL_MODE_480P;
			break;
		case 5: // 576P
			hdmode = DISPCTL_MODE_576P;
			break;
		case 6: // 720P
			hdmode = DISPCTL_MODE_720P;
			break;
		case 7:	// 1080I
			hdmode =DISPCTL_MODE_1080I;
			break;
		case 8: // 1080P
			hdmode =DISPCTL_MODE_1080P;
			break;
	    case 9: // LCD Displaying
	        hdmode = DISPCTL_MODE_480P;
			break;
		default:
			hdmode = DISPCTL_MODE_480I;
			break;

    }
    tv_output_config = (TvOutputConfig_t *)checkout_tv_output_config(hdmode);
    if (display_mode==0) // 0 for 4:3TV and 1 for 16:9TV -- please not change
	{
        tv_output_config->screen_width = 4;
        tv_output_config->screen_height = 3;
    }else{
        tv_output_config->screen_width = 16;
        tv_output_config->screen_height = 9;
    }
}



static int uVideoFmt_DisplayMode(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    int type = (int)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
    SetTVOutput(this_curVideoFmt, type);
    /* end */
    return ret;
}

static int uVideoFmt_ResetForTVOut(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    int type = (int)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
    //bootloader will check this value to select boot from which image
    if(type == 0)
    	*((volatile unsigned *)0x81000000) = 0x55aa55aa;
    else if(type == 1)
    	*((volatile unsigned *)0x81000000) = 0x00000000;

      *((volatile unsigned *)0x81000008) = 0x55aa55aa;

    /* end */
    return ret;
}

static int uVideoFmt_FLVPlayReScale(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    /* Add Method code here, Set ret to be -1 if fail */
	af_osd_layer_set_scale(OSD_LAYER_HW_LAYER0, 0, 0);
    /* end */
    return ret;
}

static int uVideoFmt_DiableVideo(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    /* Add Method code here, Set ret to be -1 if fail */
#ifdef AML_APOLLO   
    WRITE_MPEG_REG(VENC_VDAC_SETTING,0xff);
#ifdef HDMI_OUTPUT
    int hdmi_fd;
    hdmi_fd=open("/dev/hdmi",O_RDWR);
    if(hdmi_fd > 0)
    {
#ifdef CAT6611_HDMI
       ioctl(hdmi_fd, IOCMD_HDMI_SET_AV_MUTE, &Hdmi_handle_cat6611); 
   
#elif defined(EP932_HDMI)     
       ioctl(hdmi_fd, IOCMD_HDMI_SET_AV_MUTE, &Hdmi_handle_ep932); 
   
#elif defined(EP932M_HDMI)     
       ioctl(hdmi_fd, IOCMD_HDMI_SET_AV_MUTE, &Hdmi_handle_ep932m); 
   
#else
       ioctl(hdmi_fd, IOCMD_HDMI_SET_AV_MUTE, &Hdmi_handle_ep936);  
#endif  		      
       close(hdmi_fd);	
    }


#endif 		
#endif 
    /* end */
    return ret;
}

static int uVideoFmt_EnableVideo(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    /* Add Method code here, Set ret to be -1 if fail */
#ifdef AML_APOLLO   
    WRITE_MPEG_REG(VENC_VDAC_SETTING,0);
#ifdef HDMI_OUTPUT
    int hdmi_fd;
    hdmi_fd=open("/dev/hdmi",O_RDWR);
    if(hdmi_fd > 0)
    {
#ifdef CAT6611_HDMI
       ioctl(hdmi_fd, IOCMD_HDMI_CLEAR_AV_MUTE, &Hdmi_handle_cat6611); 
   
#elif defined(EP932_HDMI)     
       ioctl(hdmi_fd, IOCMD_HDMI_CLEAR_AV_MUTE, &Hdmi_handle_ep932); 
   
#elif defined(EP932M_HDMI)     
       ioctl(hdmi_fd, IOCMD_HDMI_CLEAR_AV_MUTE, &Hdmi_handle_ep932m); 
   
#else
       ioctl(hdmi_fd, IOCMD_HDMI_CLEAR_AV_MUTE, &Hdmi_handle_ep936);  
#endif  		      
       close(hdmi_fd);	
    }
#endif 		    
#endif 	 
    /* end */
    return ret;
}

static int uVideoFmt_ResizeArea(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    int layer = (int)(param[0]);
    int x = (int)(param[1]);
    int y = (int)(param[2]);
    int width = (int)(param[3]);
    int height = (int)(param[4]);
    int x_show = (int)(param[5]);
    int y_show = (int)(param[6]);
    int width_show = (int)(param[7]);
    int height_show = (int)(param[8]);
    /* Add Method code here, Set ret to be -1 if fail */
    //RecreateArea(cntl);
    osd_area_t *area;
    
    area = current_proj->resolution[af_get_current_resolution()].osd[0].area;
    if(area)
    {
        if(area->w ==960)
        {
            uvideofmt->backup_h = area->h;
            uvideofmt->backup_h_show = area->h_show;
            area->h = 200;
            area->h_show = 200;
#ifdef AML_A1H
            uvideofmt->backup_W4_reg = *(volatile unsigned *)(0xc1100000+0x1a13*4);
            *(volatile unsigned *)(0xc1100000+0x1a13*4) = area->h_show<<16|area->y_show;
#endif            
            
            
                
        }
        else if(area->w == 1280)
        {
            uvideofmt->backup_h = area->h;
            uvideofmt->backup_h_show = area->h_show;
            area->h = 150;
            area->h_show = 150;
#ifdef AML_A1H
            uvideofmt->backup_W4_reg = *(volatile unsigned *)(0xc1100000+0x1a13*4);
            *(volatile unsigned *)(0xc1100000+0x1a13*4) = area->h_show<<16|area->y_show;
#endif 
            
        }
    }
    /* end */
    return ret;
}

static int uVideoFmt_RestoreArea(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    /* Add Method code here, Set ret to be -1 if fail */
    osd_area_t *area;
    area = current_proj->resolution[af_get_current_resolution()].osd[0].area;
    if(area)
    {
        if(area->w ==960 || area->w == 1280)
        {
            area->h = uvideofmt->backup_h;
            area->h_show = uvideofmt->backup_h_show;
#ifdef AML_A1H
            *(volatile unsigned *)(0xc1100000+0x1a13*4) = uvideofmt->backup_W4_reg;
#endif 
        }
    }
    /* end */
    return ret;
}

static int uVideoFmt_VideoKeepScreen(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    int keep_flag = (int)(param[0]);
    /* Add Method code here, Set ret to be -1 if fail */
    policy_set_video_keep_screen(keep_flag);
    /* end */
    return ret;
}

static int uVideoFmt_SetOSDWindow(control_t* cntl, cond_item_t* param)
{
    int ret=0;
    uvideofmt_t* uvideofmt = (uvideofmt_t*)(cntl->private_data);
    int x = (int)(param[0]);
    int y = (int)(param[1]);
    int w = (int)(param[2]);
    int h = (int)(param[3]);
    /* Add Method code here, Set ret to be -1 if fail */
    osd_area_t *area = current_proj->resolution[af_get_current_resolution()].osd[0].area;        
    if (area){
        if (w==0)
            w = area->w_show;
        if (h==0)
            h = area->h_show;
        af_osd_set_display_window(0,x,y,w,h);
    }
    /* end */
    return ret;
}



