/*
 * CMOS Real-time Clock
 * SKELETON IMPLEMENTATION -- TO BE FILLED IN FOR TASK (1)
 */

/*
 * STUDENT NUMBER: s1810150
 */
#include <infos/drivers/timer/rtc.h>
#include <infos/define.h>
#include <infos/kernel/log.h>
#include <infos/util/list.h>
#include <infos/util/lock.h>
#include <arch/x86/pio.h>

using namespace infos::drivers;
using namespace infos::drivers::timer;
using namespace infos::arch::x86;
using namespace infos::util;
using namespace infos::kernel;

#include <infos/util/list.h>
// void out_byte(int port, int value);
// int in_byte(int port);
//#define CURRENT_CENTRY        		2000
#define CMOS_INDEX_SECOND       	0x00
#define CMOS_INDEX_MINUTE       	0x02
#define CMOS_INDEX_HOUR    		 		0x04
#define CMOS_INDEX_DAY_OF_WEEK    0x06
#define CMOS_INDEX_DAY_OF_MONTH 	0x07
#define CMOS_INDEX_MONTH        	0x08 
#define CMOS_INDEX_YEAR     			0x09 
#define CMOS_INDEX_STATUS_A 			0x0A 
#define CMOS_INDEX_STATUS_B				0x0B


class CMOSRTC : public RTC {
public:
	static const DeviceClass CMOSRTCDeviceClass;

	const DeviceClass& device_class() const override
	{
		return CMOSRTCDeviceClass;
	}

	/**
	 * Interrogates the RTC to read the current date & time.
	 * @param tp Populates the tp structure with the current data & time, as
	 * given by the CMOS RTC device.
	 */
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned int year;

  int cmos_address = 0x70;// Port 0x71
  int cmos_data    = 0x71;//data
//
	int get_update_in_progress_flag()  //helper funcition to get update situation
	{
      __outb(cmos_address, CMOS_INDEX_STATUS_A);
      return (__inb(cmos_data) & 0x80);// find the 7th bit
    }
	unsigned char get_RTC_register(int reg)//help funtion to read register
	{
      __outb(cmos_address, reg);
      return __inb(cmos_data);
    }
    //int a =__outb(0x70, 3);

	void read_timepoint(RTCTimePoint& tp) override
	{   
	    
	 	UniqueIRQLock l;     
    unsigned char registerB;
      
      // to avoid getting dodgy/inconsistent values due to RTC updates
      // wait for one round to make sure it has enough time to read from cmos

		while(!get_update_in_progress_flag());//wait for update begin 
	
		while(get_update_in_progress_flag());//wait for update finish

    second = get_RTC_register(CMOS_INDEX_SECOND);
    minute = get_RTC_register(CMOS_INDEX_MINUTE);
    hour = get_RTC_register(CMOS_INDEX_HOUR);
    day = get_RTC_register(CMOS_INDEX_DAY_OF_MONTH);
    month = get_RTC_register(CMOS_INDEX_MONTH);
    year = get_RTC_register(CMOS_INDEX_YEAR);
		
 
    registerB = get_RTC_register(0x0B);//get value from registerB 
 
    // Convert BCD to binary values if necessary
	   //binary = ((bcd / 16) * 10) + (bcd & 0xf)
    if (!(registerB & 0x04)) //get bit4 if not in binary mode( in BCD mode)
		{
      second = (second & 0x0F) + ((second / 16) * 10);//time first 4 bit by 10
      minute = (minute & 0x0F) + ((minute / 16) * 10);
      hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);//calclate the first 3bit seperately with the PM/AM indicator (first bit)
      day = (day & 0x0F) + ((day / 16) * 10);
      month = (month & 0x0F) + ((month / 16) * 10);
      year = (year & 0x0F) + ((year / 16) * 10);
    }
 
    // Convert 12 hour clock to 24 hour clock if necessary
 
    if ((!(registerB & 0x02)) && (hour & 0x80)) //12hours mode and it is pm 
		{
      if((hour & 0x7F)==12){//at 12:00pm it should do nothing 
				hour = hour;
			}
			else{
      hour = ((hour & 0x7F) + 12) % 24;//else it should add 12 then mod by 24
			}

    }
		else if((!(registerB & 0x02)) && (!(hour & 0x80)))//12hours mdoe and it is am
		{
      if((hour & 0x7F)==12){
				hour = ((hour & 0x7F) + 12) % 24;//it should convert 12:00am into 0
			}
		}
 
    //year += CURRENT_CENTRY;//add 2000 to 0019 to get correct year

		tp.day_of_month=day; //put value in the structure
		tp.hours=hour;
		tp.minutes=minute;
		tp.month=month;
		tp.seconds=second;
		tp.year=year;
		  
		// FILL IN THIS METHOD - WRITE HELPER METHODS IF NECESSARY
	}
};

const DeviceClass CMOSRTC::CMOSRTCDeviceClass(RTC::RTCDeviceClass, "cmos-rtc");

RegisterDevice(CMOSRTC);
