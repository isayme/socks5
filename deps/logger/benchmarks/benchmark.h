/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 *
 * Refers:
 *          http://nadeausoftware.com/articles/2012/03/c_c_tip_how_measure_cpu_time_benchmarking
 *          http://nadeausoftware.com/articles/2012/04/c_c_tip_how_measure_elapsed_real_time_benchmarking
 */

#ifndef __BENCHMARK_H
#define __BENCHMARK_H

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>	/* POSIX flags */
#include <sys/resource.h>
#include <time.h>	/* clock_gettime(), time() */
#include <sys/time.h>	/* gethrtime(), gettimeofday() */
#include <sys/times.h>	/* gethrtime(), gettimeofday() */

#if defined(__MACH__) && defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#else
#error "Unable to define getRealTime( ) for an unknown OS."
#endif

/**
 * Returns the real time, in seconds, or -1.0 if an error occurred.
 *
 * Time is measured since an arbitrary and OS-dependent start time.
 * The returned real time is only useful for computing an elapsed time
 * between two calls to this function.
 */
double get_real_time(void) {
#if defined(_WIN32)
	FILETIME tm;
	ULONGLONG t;
#if defined(NTDDI_WIN8) && NTDDI_VERSION >= NTDDI_WIN8
	/* Windows 8, Windows Server 2012 and later. ---------------- */
	GetSystemTimePreciseAsFileTime( &tm );
#else
	/* Windows 2000 and later. ---------------------------------- */
	GetSystemTimeAsFileTime( &tm );
#endif
	t = ((ULONGLONG)tm.dwHighDateTime << 32) | (ULONGLONG)tm.dwLowDateTime;
	return (double)t / 10000000.0;

#elif (defined(__hpux) || defined(hpux)) || ((defined(__sun__) || defined(__sun) || defined(sun)) && (defined(__SVR4) || defined(__svr4__)))
	/* HP-UX, Solaris. ------------------------------------------ */
	return (double)gethrtime( ) / 1000000000.0;

#elif defined(__MACH__) && defined(__APPLE__)
	/* OSX. ----------------------------------------------------- */
	static double timeConvert = 0.0;
	if ( timeConvert == 0.0 )
	{
		mach_timebase_info_data_t timeBase;
		(void)mach_timebase_info( &timeBase );
		timeConvert = (double)timeBase.numer /
			(double)timeBase.denom /
			1000000000.0;
	}
	return (double)mach_absolute_time( ) * timeConvert;

#elif defined(_POSIX_VERSION)
	/* POSIX. --------------------------------------------------- */
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
	{
		struct timespec ts;
#if defined(CLOCK_MONOTONIC_PRECISE)
		/* BSD. --------------------------------------------- */
		const clockid_t id = CLOCK_MONOTONIC_PRECISE;
#elif defined(CLOCK_MONOTONIC_RAW)
		/* Linux. ------------------------------------------- */
		const clockid_t id = CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_HIGHRES)
		/* Solaris. ----------------------------------------- */
		const clockid_t id = CLOCK_HIGHRES;
#elif defined(CLOCK_MONOTONIC)
		/* AIX, BSD, Linux, POSIX, Solaris. ----------------- */
		const clockid_t id = CLOCK_MONOTONIC;
#elif defined(CLOCK_REALTIME)
		/* AIX, BSD, HP-UX, Linux, POSIX. ------------------- */
		const clockid_t id = CLOCK_REALTIME;
#else
		const clockid_t id = (clockid_t)-1;	/* Unknown. */
#endif /* CLOCK_* */
		if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
			return (double)ts.tv_sec +
				(double)ts.tv_nsec / 1000000000.0;
		/* Fall thru. */
	}
#endif /* _POSIX_TIMERS */

	/* AIX, BSD, Cygwin, HP-UX, Linux, OSX, POSIX, Solaris. ----- */
	struct timeval tm;
	gettimeofday( &tm, NULL );
	return (double)tm.tv_sec + (double)tm.tv_usec / 1000000.0;
#else
	return -1.0;		/* Failed. */
#endif
}


/**
 * Returns the amount of CPU time used by the current process,
 * in seconds, or -1.0 if an error occurred.
 */
double get_cpu_time(void)
{
#if defined(_WIN32)
	/* Windows -------------------------------------------------- */
	FILETIME createTime;
	FILETIME exitTime;
	FILETIME kernelTime;
	FILETIME userTime;
	if ( GetProcessTimes( GetCurrentProcess( ),
		&createTime, &exitTime, &kernelTime, &userTime ) != -1 )
	{
		SYSTEMTIME userSystemTime;
		if ( FileTimeToSystemTime( &userTime, &userSystemTime ) != -1 )
			return (double)userSystemTime.wHour * 3600.0 +
				(double)userSystemTime.wMinute * 60.0 +
				(double)userSystemTime.wSecond +
				(double)userSystemTime.wMilliseconds / 1000.0;
	}

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
	/* AIX, BSD, Cygwin, HP-UX, Linux, OSX, and Solaris --------- */

#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0)
	/* Prefer high-res POSIX timers, when available. */
	{
		clockid_t id;
		struct timespec ts;
#if _POSIX_CPUTIME > 0
		/* Clock ids vary by OS.  Query the id, if possible. */
		if ( clock_getcpuclockid( 0, &id ) == -1 )
#endif
#if defined(CLOCK_PROCESS_CPUTIME_ID)
			/* Use known clock id for AIX, Linux, or Solaris. */
			id = CLOCK_PROCESS_CPUTIME_ID;
#elif defined(CLOCK_VIRTUAL)
			/* Use known clock id for BSD or HP-UX. */
			id = CLOCK_VIRTUAL;
#else
			id = (clockid_t)-1;
#endif
		if ( id != (clockid_t)-1 && clock_gettime( id, &ts ) != -1 )
			return (double)ts.tv_sec +
				(double)ts.tv_nsec / 1000000000.0;
	}
#endif

#if defined(RUSAGE_SELF)
	{
		struct rusage rusage;
		if ( getrusage( RUSAGE_SELF, &rusage ) != -1 )
			return (double)rusage.ru_utime.tv_sec +
				(double)rusage.ru_utime.tv_usec / 1000000.0;
	}
#endif

#if defined(_SC_CLK_TCK)
	{
		const double ticks = (double)sysconf( _SC_CLK_TCK );
		struct tms tms;
		if ( times( &tms ) != (clock_t)-1 )
			return (double)tms.tms_utime / ticks;
	}
#endif

#if defined(CLOCKS_PER_SEC)
	{
		clock_t cl = clock( );
		if ( cl != (clock_t)-1 )
			return (double)cl / (double)CLOCKS_PER_SEC;
	}
#endif

#endif

	return -1;		/* Failed. */
}

#endif // !__BENCHMARK_H
