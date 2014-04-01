#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <glib.h>

volatile unsigned long long sharedInt=0;

#define LOOPS 100000000


static __inline__ unsigned long long
rdtsc (void)
{
  unsigned long long int x;
  __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
  return x;
}
static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
                                unsigned int *ecx, unsigned int *edx)
{
        /* ecx is often an input as well as an output. */
        asm volatile("cpuid"
            : "=a" (*eax),
              "=b" (*ebx),
              "=c" (*ecx),
              "=d" (*edx)
            : "0" (*eax), "2" (*ecx));
}

unsigned long long standardIncLoop(){
  int i;
  unsigned long long start;

  unsigned eax, ebx, ecx, edx;

  eax = 1; /* processor info and feature bits */
  native_cpuid(&eax, &ebx, &ecx, &edx);
  start = rdtsc();

  for(i=0; i < LOOPS; i++){
    sharedInt++;
  }
  native_cpuid(&eax, &ebx, &ecx, &edx);
  start =  rdtsc() - start;
  printf("Finished %llu loops\n", sharedInt);
  return start;
}

unsigned long long atomicIncLoop(){
  int i;
  unsigned long long start;

  unsigned eax, ebx, ecx, edx;

  eax = 1; /* processor info and feature bits */
  native_cpuid(&eax, &ebx, &ecx, &edx);
  start = rdtsc();

  for(i=0; i < LOOPS; i++){
    g_atomic_int_inc(&sharedInt);
  }
  sharedInt++;
  native_cpuid(&eax, &ebx, &ecx, &edx);
  sharedInt--;
  start =  rdtsc() - start;
  printf("Finished %llu loops\n", sharedInt);
  return start;
}

int main()
{
  unsigned long long time;
  unsigned long long timeFunc;

  printf("Running standard increment loop %d times\n", LOOPS);
  timeFunc = rdtsc();
  time = standardIncLoop();
  timeFunc = rdtsc() - timeFunc;
  printf("Cycles per iteration: %f  cycles for function: %f\n", ((float)time)/LOOPS, ((float)timeFunc)/LOOPS);
  printf("Cycles: %llu  cycles for function: %llu\n", time, timeFunc);

  printf("Running atomic increment loop %d times\n", LOOPS);
  timeFunc = rdtsc();
  time = atomicIncLoop();
  timeFunc = rdtsc() - timeFunc;
  printf("Cycles per iteration: %f  cycles for function: %f\n", ((float)time)/LOOPS, ((float)timeFunc)/LOOPS);
  printf("Cycles: %llu  cycles for function: %llu\n", time, timeFunc);


}
