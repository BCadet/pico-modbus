#include "RV3028/RV3028.h"
#include "platform.h"
#include <stdio.h>

#define PASSWORD		0x20

static struct tm CurrentTime = 
{
    .tm_sec = 0,
    .tm_min = 40,
    .tm_hour = 17,
    .tm_wday = 2,
    .tm_mday = 28,
    .tm_mon = 02,
    .tm_year = 23,
};

static rv3028_error_t ErrorCode;
static rv3028_t RTC;
static rv3028_init_t RTC_Init = {
    // Use this settings to enable the battery backup
    .BatteryMode = RV3028_BAT_DSM,
    .Resistance = RV3028_TCT_3K,
    .EnableBSIE = true,
    .EnableCharge = true,

    // Use this settings to configure the time stamp function
    //.TSMode = RV3028_TS_BAT,
    .EnableTS = true,
    //.EnableTSOverwrite = true,

    // Use this settings to enable the clock output
    .Frequency = RV3028_CLKOUT_8KHZ,
    .EnableClkOut = true,

    // Use this settings for the event input configuration
    .EnableEventInt = true,
    .EventHighLevel = false,
    .Filter = RV3028_FILTER_256HZ,

    // Set the current time
    .HourMode = RV3028_HOURMODE_24,
    .p_CurrentTime = &CurrentTime,

    // Use this settings for the Power On Reset interrupt
    .EnablePOR = false,

    // Use this settings for the password function
    .Password = PASSWORD,
};

void rtc_init(void)
{
    platform_rv3028_init();

    RTC.p_Read = platform_rv3028_read;
    RTC.p_Write = platform_rv3028_write;
    RTC.DeviceAddr = RV3028_ADDRESS;

	ErrorCode = RV3028_Init(&RTC_Init, &RTC);
    if(ErrorCode == RV3028_NO_ERROR)
	{
	    printf("\r\nRTC initialized...");
	    printf("\r\n  HID: %x", RTC.HID);
	    printf("\r\n  VID: %x", RTC.VID);
    }
}

void rtc_update(void)
{
    RV3028_GetTime(&RTC, &CurrentTime);
}

void* rtc_get_current_time(void)
{
    return &CurrentTime;
}