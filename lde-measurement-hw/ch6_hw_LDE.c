#include <bits/time.h>
#include <sched.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#define ONE_HUNDRED_MILLION 100000000LL
#define BILLION 1000000000LL
#define NUM_SWITCHES 1000000

int localpid(void) {
  static int a[9] = { 0 };
  return a[0];
}

void print_ns_us_ms(const char* label, uint64_t delta_time) { 
  printf("%s%llu ns = %.3f us = %.6f ms\n", label, (unsigned long long) delta_time, delta_time / 1000.0, delta_time / 1.0e6);
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
    printf("i ticks: %llu, j ticks: %llu, j - i ticks: %llu\n", (unsigned long long) i, (unsigned long long) j, (unsigned long long) (j - i));
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

uint64_t measure_system_call(clockid_t clockid, void* (*fp)(void*), void* args) { 
  struct timespec start, end;
  // Get the start time
  clock_gettime(clockid, &start);

  // Call the function
  fp(args);

  // Get the end time
  clock_gettime(clockid, &end);

  // Calculate elapsed time in miliseconds
  uint64_t diff = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);
  return diff;
}

// System call wrapper to pass to measure_system_call
void* getpid_wrapper(void* args) {
  (void) args;  // args are unused, this silences the compiler warning
  getpid();  // System call
  return NULL;
}

volatile int switch_control = 0;

// Thread 1: Yields to Thread 2
void* thread1_func(void* args) {
  for (int i = 0; i < NUM_SWITCHES; i++) {
    while (switch_control != 0);  // Busy wait for control
    switch_control = 1;  // Pass control to thread 2
    sched_yield();  // Force context switch
  }
  return NULL;
}

// Thread 2: Yields to Thread 1
void* thread2_func(void* args) {
  for (int i = 0; i < NUM_SWITCHES; i++) {
    while (switch_control != 1);  // Busy wait for control
    switch_control = 0;  // Pass control to thread 1
    sched_yield();  // Force context switch
  }
  return NULL;
}

uint64_t measure_context_switching(clockid_t clockid, void* (*fp)(void*), void* args) { 
  pthread_t thread1, thread2;
  struct timespec start, end;

  // Create two threads
  pthread_create(&thread1, NULL, thread1_func, NULL);
  pthread_create(&thread2, NULL, thread2_func, NULL);

  // Start time measurement
  clock_gettime(clockid, &start);

  // Wait for both threads to finish
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  // End time measurement
  clock_gettime(clockid, &end);

  // Calculate elapsed time in nanoseconds
  uint64_t elapsed_time = (end.tv_sec - start.tv_sec) * BILLION + (end.tv_nsec - start.tv_nsec);

  return elapsed_time;
}

int main(int argc, char **argv) {
  measure_generic("elapsed time = ", CLOCK_MONOTONIC, (void *)sleep, (void *)1L);

  /* the time spent sleeping will not count (but there is a bit of overhead */
  measure_generic("elapsed process CPU time = ", CLOCK_PROCESS_CPUTIME_ID, (void *)sleep, (void*)1L);

  measure_generic("elapsed process CPU time for gettimeofday() = ", CLOCK_PROCESS_CPUTIME_ID, (void *)repeat_get_time_of_day, NULL);

  repeat_rdtsc();
  repeat_rdtsc();

  uint64_t elapsed = measure_system_call(CLOCK_MONOTONIC, getpid_wrapper, NULL);
  printf("Elapsed time for getpid() system call: %llu ns\n", (unsigned long long) elapsed);

  elapsed = measure_context_switching(CLOCK_MONOTONIC, NULL, NULL);
  printf("Elapsed time for context switching: %llu ns\n", (unsigned long long) elapsed);

  exit(0);
  return 0;
}

