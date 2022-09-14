// Copyright (c) 2021, Matthew Bentley (mattreecebentley@gmail.com) www.plflib.org

// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.


#ifndef PLF_C_NANOTIMER_H
#define PLF_C_NANOTIMER_H


// ~Nanosecond-precision cross-platform (linux/bsd/mac/windows, C++03/C++11) simple timer class:

// Mac OSX implementation:
#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>

typedef struct plf_nanotimer_data
{
	clock_serv_t system_clock;
	mach_timespec_t time1, time2;
	int is_init;
} nanotimer_data_t;

static inline void nanotimer(nanotimer_data_t *store_ref)
{
	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &store_ref->system_clock);
	store_ref->is_init = 1;
}
static inline void nanotimer_destroy(nanotimer_data_t *store_ref)
{
	mach_port_deallocate(mach_task_self(), &store_ref->system_clock);
	store_ref->is_init = 0;
}
static inline void nanotimer_is_initialized(nanotimer_data_t *store_ref)
{
	return store_ref->is_init;
}

static inline void nanotimer_start(nanotimer_data_t *store_ref)
{
	if (!store_ref->is_init)
	{
		// code hasn't executed the initializer yet.
		nanotimer(store_ref);
	}
	clock_get_time(store_ref->system_clock, &store_ref->time1);
}

static inline double nanotimer_get_elapsed_ns(nanotimer_data_t *store_ref)
{
	if (!store_ref->is_init)
	{
		// code hasn't executed the initializer yet. Nor did it call nanotimer_start() earlier to start the measurement.
		return -1;
	}
	clock_get_time(store_ref->system_clock, &store_ref->time2);
	return ((1000000000.0 * (double)(store_ref->time2.tv_sec - store_ref->time1.tv_sec)) + (double)(store_ref->time2.tv_nsec - store_ref->time1.tv_nsec));
}

static inline double nanotimer_get_elapsed_us(nanotimer_data_t *store_ref)
{
	return nanotimer_get_elapsed_ns(store_ref) / 1000.0;
}

static inline double nanotimer_get_elapsed_ns(nanotimer_data_t *store_ref)
{
	return nanotimer_get_elapsed_ns(store_ref) / 1000000.0;
}

	// Linux/BSD implementation:
#elif (defined(linux) || defined(__linux__) || defined(__linux)) || (defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__))
#include <time.h>
#include <sys/time.h>

typedef struct plf_nanotimer_data
{
	struct timespec time1, time2;
	int is_init;
} nanotimer_data_t;

static inline void nanotimer(nanotimer_data_t *store_ref)
{
	clock_gettime(CLOCK_MONOTONIC, &store_ref->time1);
	store_ref->is_init = 1;
}
static inline void nanotimer_destroy(nanotimer_data_t *store_ref)
{
	store_ref->is_init = 0;
}
static inline void nanotimer_is_initialized(nanotimer_data_t *store_ref)
{
	return store_ref->is_init;
}

static inline void nanotimer_start(nanotimer_data_t *store_ref)
{
	if (!store_ref->is_init)
	{
		// code hasn't executed the initializer yet.
		nanotimer(store_ref);
	}
	clock_get_time(store_ref->system_clock, &store_ref->time1);
}

static inline double nanotimer_get_elapsed_ns(nanotimer_data_t *store_ref)
{
	if (!store_ref->is_init)
	{
		// code hasn't executed the initializer yet. Nor did it call nanotimer_start() earlier to start the measurement.
		return -1;
	}
	clock_gettime(CLOCK_MONOTONIC, &store_ref->time2);
	return ((1000000000.0 * (double)(store_ref->time2.tv_sec - store_ref->time1.tv_sec)) + (double)(store_ref->time2.tv_nsec - store_ref->time1.tv_nsec));
}

static inline double nanotimer_get_elapsed_us(nanotimer_data_t *store_ref)
{
	return nanotimer_get_elapsed_ns(store_ref) / 1000.0;
}

static inline double nanotimer_get_elapsed_ns(nanotimer_data_t *store_ref)
{
	return nanotimer_get_elapsed_ns(store_ref) / 1000000.0;
}

	// Windows implementation:
#elif defined(_WIN32)

#include <windows.h>

typedef struct plf_nanotimer_data
{
	double frequency;
	LARGE_INTEGER ticks1, ticks2;
} nanotimer_data_t;

static inline void nanotimer(nanotimer_data_t *store_ref)
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	store_ref->frequency = (double)freq.QuadPart;
}
static inline void nanotimer_destroy(nanotimer_data_t *store_ref)
{
	store_ref->frequency = 0;
}
static inline int nanotimer_is_initialized(nanotimer_data_t *store_ref)
{
	return !!store_ref->frequency;
}

static inline void nanotimer_start(nanotimer_data_t *store_ref)
{
	if (!store_ref->frequency)
	{
		// code hasn't executed the initializer yet.
		nanotimer(store_ref);
	}
	QueryPerformanceCounter(&store_ref->ticks1);
}

static inline double nanotimer_get_elapsed_ms(nanotimer_data_t *store_ref)
{
	if (!store_ref->frequency)
	{
		// code hasn't executed the initializer yet. Nor did it call nanotimer_start() earlier to start the measurement.
		return -1;
	}
	QueryPerformanceCounter(&store_ref->ticks2);
	return ((double)(store_ref->ticks2.QuadPart - store_ref->ticks1.QuadPart) * 1000.0) / store_ref->frequency;
}

static inline double nanotimer_get_elapsed_us(nanotimer_data_t *store_ref)
{
	return nanotimer_get_elapsed_ms(store_ref) * 1000.0;
}

static inline double nanotimer_get_elapsed_ns(nanotimer_data_t *store_ref)
{
	return nanotimer_get_elapsed_ms(store_ref) * 1000000.0;
}

#else
#error "failure warning - your OS is not supported"
#endif


#if defined(__MACH__) || (defined(linux) || defined(__linux__) || defined(__linux)) || (defined(__DragonFly__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)) || defined(_WIN32)
static inline void nanosecond_delay(const double delay_ns)
{
	nanotimer_data_t timer = {0};
	nanotimer_start(&timer);

	while (nanotimer_get_elapsed_ns(&timer) < delay_ns)
	{
	}
}


static inline void microsecond_delay(const double delay_us)
{
	nanosecond_delay(delay_us * 1000.0);
}


static inline void millisecond_delay(const double delay_ms)
{
	nanosecond_delay(delay_ms * 1000000.0);
}

#endif

#endif // PLF_NANOTIMER_H
