#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

/* This code shows how to create a new file and then mmap it to put data
   inside of it.  It creates the file in /hugepages, which should be where
   you have setup your hugepagefs.
   Running grep Huge /proc/meminfo should show that the equivalent of 20MB
   of space has been used by the program.
   When you are done delete the file manually.

   Original code:
   http://www.linuxquestions.org/questions/programming-9/mmap-tutorial-c-c-511265/
*/

#define FILEPATH "/hugepages/mmapped.bin"
#define FILESIZE 1024*1024*2*10
#define NUMINTS FILESIZE/sizeof(int) -1

int main(int argc, char *argv[])
{
    int i;
    int fd;
    int result;
    int *map;  /* mmapped array of int's */

    /* Open a file for writing.
     *  - Creating the file if it doesn't exist.
     *  - Truncating it to 0 size if it already exists. (not really needed)
     *
     * Note: "O_WRONLY" mode is not sufficient when mmaping.
     */
    fd = open(FILEPATH, O_CREAT | O_RDWR, 0755);
    if (fd == -1) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    /* Now the file is ready to be mmapped.
     */
    map = mmap(0, FILESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    /* Now write int's to the file as if it were memory (an array of ints).
     */
    for (i = 1; i <=NUMINTS; ++i) {
        map[i] = 2 * i;
    }

    /* Don't forget to free the mmapped memory
     */
    if (munmap(map, FILESIZE) == -1) {
        perror("Error un-mmapping the file");
        /* Decide here whether to close(fd) and exit() or not. Depends... */
    }

    /* Un-mmaping doesn't close the file, so we still need to do that.
     */
    close(fd);
    return 0;
}
