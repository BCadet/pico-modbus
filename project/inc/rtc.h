#ifndef RTC_H
#define RTC_H
#pragma once

void rtc_init(void);
void* rtc_get_current_time(void);
void rtc_update(void);

#endif 