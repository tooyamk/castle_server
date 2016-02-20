#pragma once

#include <windows.h>
#include <time.h>

class Helper {
public:
	Helper();
	virtual ~Helper();

	static inline void itimeofday(long *sec, long *usec) {
#if defined(__unix)
		struct timeval time;
		gettimeofday(&time, NULL);
		if (sec) *sec = time.tv_sec;
		if (usec) *usec = time.tv_usec;
#else
		static long mode = 0, addsec = 0;
		BOOL retval;
		static long long freq = 1;
		long long qpc;
		if (mode == 0) {
			retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
			freq = (freq == 0) ? 1 : freq;
			retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
			addsec = (long)time(NULL);
			addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
			mode = 1;
		}
		retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
		retval = retval * 2;
		if (sec) *sec = (long)(qpc / freq) + addsec;
		if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
#endif
	}

	static inline long long iclock64(void) {
		long s, u;
		long long value;
		itimeofday(&s, &u);
		value = ((long long)s) * 1000 + (u / 1000);
		return value;
	}

	static inline unsigned long iclock() {
		return (unsigned long)(iclock64() & 0xfffffffful);
	}
};