#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <glib.h>

/*
  This is test code to measure the time of different kinds
  of concurrent data accesses.  Right now it is unfinished
  and probably does not work correctly.

  OS X doesn't support CPU affinity, so sometimes a thread
  switches between cores. This breaks rtdsc calls.

  When using a large number of iterations, the atomic timing
  tests seem to be messed up. Sometimes the cycles per op
  becomes very low (less than non-atomic). Suspicious...

  Author: timwood@gwu.edu
*/

#define LOOPS 20000000
#define NUMTHREADS 2
#define ARRAYSIZE 1024
#define SPACER ARRAYSIZE/(NUMTHREADS)

// Should this be volatile???
unsigned long long gSharedInt[ARRAYSIZE];

#define THREAD_UNSAFE 0
#define THREAD_ATOMIC 1
int gThreadMode = THREAD_ATOMIC;

/* Use the CPU cycle counter as a timer */
static __attribute__((always_inline)) unsigned long long
rdtsc (void)
{
  unsigned long long int x;
  __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
  return x;
}
/* Get the CPUID info. This forces the CPU to clear out the
  pipeline, preventing rdtsc from running before other instructions.
*/
static __attribute__((always_inline)) void
native_cpuid(unsigned int *eax, unsigned int *ebx,
                                unsigned int *ecx, unsigned int *edx)
{
  __asm__ volatile("cpuid"
      : "=a" (*eax),
        "=b" (*ebx),
        "=c" (*ecx),
        "=d" (*edx)
      : "0" (*eax), "2" (*ecx));
}
/* Combine both rdtsc and cpuid to make a safe time checker */
static __attribute__((always_inline)) unsigned long long
syncrdtsc (void)
{
  unsigned int eax, ebx, ecx, edx;
  eax = 1;
  unsigned long long int t;
  native_cpuid(&eax, &ebx, &ecx, &edx);
  t = rdtsc();
  return t;
}

/* Update a shared global variable with no locking */
unsigned long long standardIncLoop(unsigned long long * sharedData){
  int i;
  unsigned long long timer;

  timer = syncrdtsc();

  for(i=0; i < LOOPS; i++){
    (*sharedData)++;
  }
  timer =  syncrdtsc() - timer;
  return timer;
}

/* Update a shared global variable with glib atomic update instruction */
unsigned long long atomicIncLoop(unsigned long long * sharedData){
  int i;
  unsigned long long timer;

  timer = syncrdtsc();

  for(i=0; i < LOOPS; i++){
    g_atomic_int_inc(sharedData);
  }
  timer =  syncrdtsc() - timer;
  return timer;
}

// TODO: add code for pthread mutex inc updates

/* Measure the cycles needed to sleep for 1 second */
unsigned long long timedsleep(int millisleep) {
  unsigned long long timer;
  timer = syncrdtsc();
  usleep(millisleep*1000);
  timer =  syncrdtsc() - timer;
  return timer;
  // TODO: use this to calibrate the rdtsc timer

}

/* Body of a new thread. */
void* threadIncLoop(void *i){
  unsigned long long timer;
  int *threadID = (int*)i;

  if(gThreadMode == THREAD_ATOMIC) {
    printf("Thread %d starting loop with atomic ops\n", *threadID);
    timer = atomicIncLoop(&gSharedInt[*threadID * SPACER]);
  }
  else if(gThreadMode == THREAD_UNSAFE) {
    printf("Thread %d starting loop with unsafe ops\n", *threadID);
    timer = standardIncLoop(&gSharedInt[*threadID * SPACER]);
  }
  else {
    printf("OOPS, not implemented yet...\n");
  }

  printf("Thread %d Cycles/op: %f\n", *threadID, ((float)timer)/LOOPS);
  printf("Counter = %llu\n", gSharedInt[0]);
  pthread_exit(NULL);
}

int main()
{
  unsigned long long timer;
  pthread_t threads[NUMTHREADS];
  int i;
  int ids[NUMTHREADS];

  // Intel doc says to run CPUID > 3 times at timer since it may be slow
  // the first few times you run it.
  printf("Warming up...\n");
  standardIncLoop(&gSharedInt[0]);
  standardIncLoop(&gSharedInt[0]);
  standardIncLoop(&gSharedInt[0]);

  printf("==== SINGLE THREADED ====\n");

/*
  printf("Time to sleep 1 second...\n");
  timer = timedsleep(1000);
  printf("Cycles for 1000 msec sleep: %llu\n", timer);
*/
  printf("Running standard increment loop %d times\n", LOOPS);
  gSharedInt[0] = 0;
  timer = standardIncLoop(&gSharedInt[0]);
  printf("Cycles per iteration: %f\n", ((float)timer)/LOOPS);

  printf("Running atomic increment loop %d times\n", LOOPS);
  gSharedInt[0] = 0;
  timer = atomicIncLoop(&gSharedInt[0]);
  printf("Cycles per iteration: %f\n", ((float)timer)/LOOPS);
  printf("Single-Threaded Total: %llu cycles\n", timer);

  printf("==== MULTI THREADED ====\n");
  gThreadMode = THREAD_ATOMIC;
  timer = syncrdtsc();
  gSharedInt[0] = 0;
  for(i=0; i < NUMTHREADS; i++){
    ids[i] = i;
    if (pthread_create(&threads[i], NULL, threadIncLoop, &ids[i])) {
        perror("pthread_create: ");
        exit(1);
    }
  }

  for (i = 0; i < NUMTHREADS; i++){
    pthread_join(threads[i], NULL);
  }
  printf("Expected Counter: %d\n", LOOPS*NUMTHREADS);
  timer = syncrdtsc() - timer;
  printf("Multi-Threaded Total: %llu cycles\n", timer);

}
