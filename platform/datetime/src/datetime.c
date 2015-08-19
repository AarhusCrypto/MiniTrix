#include <datetime.h>
#include <osal.h>
#include <coov4.h>
#include <common.h>
#ifdef OSX
#include <mach/mach_time.h>
#elif WINDOWS
#include <Windows.h>
#include <time.h>
#else
#include <time.h>
#endif

COO_DEF(DateTime, ull, getMicroTime) 
	return this->getNanoTime()/1000;
}


COO_DEF(DateTime,ull,getMilliTime) 
	return this->getNanoTime()/1000000;
}


COO_DEF(DateTime,uint,getSecondTime)
	return (uint)this->getNanoTime()/1000000000L;
}


COO_DEF(DateTime, ull, getNanoTime)
#ifdef OSX
return mach_absolute_time();
#elif WINDOWS
/*
   GetSystemTimeAsFileTime primitive used below
   reports back the number of 100-nanoseconds elapsed 
   since january 1st 1601.

   according to: http://stackoverflow.com/questions/1695288/getting-the-current-time-in-milliseconds-from-the-system-clock-in-windows
   this discussion this is one of the more reliable ways of getting fine precision time
   on windows. Yes I know the {getNanoTime} function is kind of inaccurate now.
 */
FILETIME t = { 0 };
ull result = 0;
GetSystemTimeAsFileTime(&t);
result = t.dwLowDateTime;
result += (((unsigned long long)t.dwHighDateTime) << 32);
return (result - 116444736000000000LL)*100;
#else
	// clock_gettime requires -lrt
  struct timespec tspec = {0};
  if (clock_gettime(CLOCK_REALTIME,&tspec) == 0) {
    return 1000000000L*tspec.tv_sec + tspec.tv_nsec;
  } else {
    return 0;
  }
#endif
}


DateTime DateTime_New(OE oe) {
	DateTime res = oe->getmem(sizeof(*res));
	res->getNanoTime = COO_attach(res, DateTime_getNanoTime);
	res->getMicroTime = COO_attach(res, DateTime_getMicroTime);
	res->getSecondTime = COO_attach(res, DateTime_getSecondTime);
	res->getMilliTime = COO_attach(res, DateTime_getMilliTime);
	res->impl = oe;
	return res;
}

void Register_DatetimeDefaultConstructor(OE oe) {
	oe->provideSystemLibrary(DATE_TIME_LIBRARY,(DefaultConstructor)DateTime_New);
}

void DateTime_Destroy(DateTime * instance) {
	DateTime dt = 0;
	OE oe = 0;
	if (!instance) return;

	dt = *instance;
	oe = (OE)dt->impl;

	COO_detach(dt->getNanoTime);
	COO_detach(dt->getMicroTime);
	COO_detach(dt->getMilliTime);
	COO_detach(dt->getSecondTime);
	oe->putmem(dt);
	*instance = 0;
}
