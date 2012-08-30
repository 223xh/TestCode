#include "includes.h"
#include "ioapi.h"
#include <Drivers/rtc/rtc_drv.h>

/********************************
vflag:0,Disable adjust.1,Enable adjust.

mflag:monitor_use_adjusted. For debugging purpose.
0 = sec_pulse_out outputs un-adjusted second pulse;
1 = sec_pulse_out outputs adjusted second pulse.

gflag:gpo_use_adjusted.
0 = gpo_counter uses un-adjusted time tick;
1 = gpo_counter uses adjusted time tick.

adflag:sec_adjust_ctrl. CPU command to control time adjustment.
00 = No adjustment;
10(2) = Swallow a second once every ad_secs+1 seconds;
11(3) = Insert a second once every ad_secs+1 seconds.

ad_secs:match_counter. In the unit of second. Once every match_counter+1 seconds, a second pulse is to be swallowed/inserted.MAX 0x7ffff(524287).
********************************/
void InternalRTCSetAdjust(unsigned char vflag, unsigned char mflag, unsigned gflag, unsigned char adflag, unsigned ad_secs)//
{
    int ifd = -1;
    unsigned adjust_val = 0;
    
    ifd = open("/dev/rtc_dev", O_RDWR);
    if(ifd)
    {        
        adjust_val = ((vflag&1)<<23) | ((mflag&1)<<22) | ((gflag&1)<<21) | ((adflag&3)<<19) | (adsecs&0x7ffff);
        ioctl(ifd, RTC_SET_ADJUST, adjust_val);
        close(ifd);
    }
}