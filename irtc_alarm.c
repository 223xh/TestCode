#include "includes.h"
#include "ioapi.h"
#include <Drivers/rtc/rtc_drv.h>

unsigned GetGPO()//1=HIGH, 0 = LOW
{
    unsigned level = 0;
    int ifd = -1;
    
    ifd = open("/dev/rtc_dev", O_RDWR);
    if(ifd)
    {
        ioctl(ifd, RTC_GET_GPOLEVEL, &level);
        close(ifd);
    }
    
    return !level;
}

void SetGPO(unsigned level)//1=HIGH, 0 = LOW
{
    int ifd = -1;
    
    ifd = open("/dev/rtc_dev", O_RDWR);
    if(ifd)
    {
        ioctl(ifd, RTC_RESET_GPOLEVEL, level);
        close(ifd);
    }
}

void SetAlarm(unsigned secs)//After secs seconds GPO is set to low level.secs<1048576
{
    int ifd = -1;
    unsigned alarm_val;
    
    ifd = open("/dev/rtc_dev", O_RDWR);
    if(ifd)
    {
        alarm_val = (1<<22) | (2<<20) | (secs&0xfffff);
        ioctl(ifd, RTC_SET_ALARM, alarm_val);
        close(ifd);
    }
}