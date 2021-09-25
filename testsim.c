/**
 * TestSim Application:
 * - Takes 2 CLI arguments: Sleep time and Repeat factor
 *    --> Repeat factor is the number of times "testsim" iterates a loop
 *   --> Sleep time is the number of seconds between each iteration
 * - In each iteration of the loop, "testsim" sleeps for the specified amount of time, then outputs a message with its `pid` to logfile using logmsg in the format
 */

#include <stdio.h>
#include <stdlib.h>

void deallocateSharedMemory(int shmid);

int main(int argc, char** argv) {
  // Read, validate CLI arguments

  // Set up shared memory

  return 0;
}

// A Function to Deallocate Shared Memory
void deallocateSharedMemory(int shmid) {
  // Deallocate shared memory

}