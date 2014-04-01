#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <glib.h>

volatile unsigned long long sharedInt=0;

#define LOOPS 10000000

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
  /* ecx is often an input as well as an output. */
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
  //native_cpuid(&eax, &ebx, &ecx, &edx);
  t = rdtsc();
  return t;
}

/* Update a shared global variable with no locking */
unsigned long long standardIncLoop(){
  int i;
  unsigned long long timer;

  sharedInt = 0;

  timer = syncrdtsc();

  for(i=0; i < LOOPS; i++){
    sharedInt++;
  }
  timer =  syncrdtsc() - timer;
  printf("Finished %llu loops\n", sharedInt);
  return timer;
}

/* Update a shared global variable with glib atomic update instruction */
unsigned long long atomicIncLoop(){
  int i;
  unsigned long long timer;

  sharedInt = 0;

  timer = syncrdtsc();

  for(i=0; i < LOOPS; i++){
    g_atomic_int_inc(&sharedInt);
  }
  timer =  syncrdtsc() - timer;
  printf("Finished %llu loops\n", sharedInt);
  return timer;
}

/* Measure the cylces needed to sleep for 1 second */
unsigned long long timedsleep(int millisleep) {
  unsigned long long timer;
  timer = syncrdtsc();
  usleep(millisleep*1000);
  timer =  syncrdtsc() - timer;
  return timer;

}

int main()
{
  unsigned long long time;
  unsigned long long timeFunc;

  // Intel doc says to run CPUID > 3 times at timer since it may be slow
  // the first few times you run it.
  printf("Trial runs (not timed)...\n");
  standardIncLoop();
  standardIncLoop();
  standardIncLoop();

  printf("Time to sleep 1 second...\n");
  time = timedsleep(1000);
  printf("Cycles for 1000 msec sleep: %llu\n", time);

  printf("Running standard increment loop %d times\n", LOOPS);
  time = standardIncLoop();
  printf("Cycles per iteration: %f\n", ((float)time)/LOOPS);

  printf("Running atomic increment loop %d times\n", LOOPS);
  time = atomicIncLoop();
  printf("Cycles per iteration: %f\n", ((float)time)/LOOPS);


}
