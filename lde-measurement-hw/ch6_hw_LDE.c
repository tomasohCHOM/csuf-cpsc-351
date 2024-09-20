/*
 * gettime - get time via clock_gettime
 * N.B.: OS X does not support clock_gettime
 *
 * Paul Krzyzanowski
*/

#include <stdio.h>	/* for printf */
#include <stdint.h>	/* for uint64 definition */
#include <stdlib.h>	/* for exit() definition */
#include <time.h>	/* for clock_gettime */
#include <unistd.h> // for sleep
#include <sys/time.h>

#define ONE_HUNDRED_MILLION 100000000L
#define BILLION 1000000000L

int localpid(void) {
  static int a[9] = { 0 };
  return a[0];
}

void print_ns_us_ms(const char* label, uint64_t delta_time) { 
  printf("%s%llu ns = %.3f us = %.6f ms\n", label, delta_time, delta_time / 1000.0, delta_time / 1.0e6);
}

#define REPEATS ONE_HUNDRED_MILLION

// rdtsc.cpp
// processor: x86, x64
// #include <intrin.h>  // INTEL ONLY (will not run on ARM processors like M1, M2, M3)

// #pragma intrinsic(__rdtsc)

uint64_t repeat_rdtsc(void) { 
  uint64_t i, j;
  for (uint64_t k = 0; k < 5; ++k) { 
    i = __rdtsc();
    j = __rdtsc();
    printf("i ticks: %llu, j ticks: %llu, j - i ticks: %llu\n", i, j, j - i);
    print_ns_us_ms("rtdsc = ", j - i);
  }
  return REPEATS;
}

uint64_t repeat_get_time_of_day(void) { 
  struct timeval time;
  for (uint64_t i = 0; i < REPEATS; ++i) { 
    gettimeofday(&time, 0);   // Note: won't ACTUALLY call this system call everytime, maybe 1000 / 100_000_000 (1 out of 100,000)
  }                             //    will use cached memory because resolution of gettimeofday() is so low -- it doesn't change often
  return REPEATS;
}

uint64_t measure_generic(const char* label, clockid_t clockid, void* (*fp)(void*), void* args) { 
  uint64_t diff;
  struct timespec start, end;

  clock_gettime(clockid, &start);	/* mark start time */

  uint64_t repeats = (uint64_t)fp(args);  // allows for some processes to run multiple times; others only once

  clock_gettime(clockid, &end);	/* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
  // printf("diff in measure_generic is: %llu\n", diff);
  if (repeats == REPEATS) { diff /= REPEATS; }   
  print_ns_us_ms(label, diff);

  return diff;
}

// llui_t measure_system_call(clockid_t clockid, void* (*fp)(void* args)) { 
//   fp(args);
//   return 0LLU;
// }

// llui_t measure_context_switch() { 
//   return 0LLU;
// }

int main(int argc, char **argv) {
  measure_generic("elapsed time = ", CLOCK_MONOTONIC, (void *)sleep, (void *)1L);

  /* the time spent sleeping will not count (but there is a bit of overhead */
  measure_generic("elapsed process CPU time = ", CLOCK_PROCESS_CPUTIME_ID, (void *)sleep, (void*)1L);

  measure_generic("elapsed process CPU time for gettimeofday() = ", CLOCK_PROCESS_CPUTIME_ID, (void *)repeat_get_time_of_day, NULL);

  repeat_rdtsc();
  repeat_rdtsc();

  exit(0);
  return 0;
}

