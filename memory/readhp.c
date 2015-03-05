/*
 * This is based on the linux example: hugepage-shm
 *
 * This simply reads through a previously allocated huge page
 * and verifies that it has been written to as defined in the
 * writehp program.
 * This should be run after writehp.c is started (but before it exits)
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>

#ifndef SHM_HUGETLB
#define SHM_HUGETLB 04000
#endif

#define LENGTH (2UL*1024*1024)

#define dprintf(x)  printf(x)

/* Only ia64 requires this */
#ifdef __ia64__
#define ADDR (void *)(0x8000000000000000UL)
#define SHMAT_FLAGS (SHM_RND)
#else
#define ADDR (void *)(0x0UL)
#define SHMAT_FLAGS (0)
#endif

/* The "key" for the requested hugepage */
#define KEY 123

int main(void)
{
        int shmid;
        unsigned long i;
        char *shmaddr;
        int num;

        shmid = shmget(KEY, LENGTH, SHM_HUGETLB | SHM_R | SHM_W);
        if (shmid < 0) {
                perror("shmget");
                exit(1);
        }
        printf("shmid: 0x%x\n", shmid);

        shmaddr = shmat(shmid, ADDR, SHMAT_FLAGS);
        if (shmaddr == (char *)-1) {
                perror("Shared memory attach failure");
                shmctl(shmid, IPC_RMID, NULL);
                exit(2);
        }
        printf("shmaddr: %p\n", shmaddr);

        dprintf("Starting the Check...");
        for (i = 0; i < LENGTH; i++)
                if (shmaddr[i] != (char)i) {
                        printf("\nIndex %lu mismatched\n", i);
                        exit(3);
                }
        dprintf("Done.\n");

        if (shmdt((const void *)shmaddr) != 0) {
                perror("Detach failure");
                shmctl(shmid, IPC_RMID, NULL);
                exit(4);
        }

        shmctl(shmid, IPC_RMID, NULL);

        return 0;
}
