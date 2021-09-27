/**
 * TestSim Application:
 * - Takes 2 CLI arguments: Sleep time and Repeat factor
 *    --> Repeat factor is the number of times "testsim" iterates a loop
 *   --> Sleep time is the number of seconds between each iteration
 * - In each iteration of the loop, "testsim" sleeps for the specified amount of time, then outputs a message with its `pid` to logfile using logmsg in the format
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "config.h"
#include "license.h"

void deallocateSharedMemory(int shmid);

int main(int argc, char** argv) {
  printf("In testsim! Args:\n");
  // Read, validate CLI arguments
  // print args
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  printf("\n");

  // validate cli arguments
  if (argc != 3) {
    printf("Error: Invalid number of arguments.\n");
    fprintf(stderr, "Usage: testsim <sleep time> <repeat factor>\n");
    return 1;
  }

  // convert cli arguments to ints
  int sleepTime = atoi(argv[1]);
  int repeatFactor = atoi(argv[2]);

  // validate cli arguments
  if (sleepTime < 0 || repeatFactor < 0) {
    printf("Error: Invalid arguments. Must be positive integers\n");
    fprintf(stderr, "Usage: testsim <sleep time> <repeat factor>\n");
    return 1;
  }

  // Enter loop for number of iterations
  for (int i = 0; i < repeatFactor; i++) {

    // sleep for sleepTime seconds
    printf("Sleeping for %d seconds\n", sleepTime);
    sleep(sleepTime);
    // generate char* message of the time, pid, iteration #
    char* message = malloc(sizeof(char) * (strlen(argv[0]) + strlen(argv[1]) + strlen(argv[2]) + strlen(argv[3]) + 10));
    printf("message: %s\n", message);
    // sprintf(message, "%s %s %s %d", argv[0], argv[1], argv[2], i);
    // log message
    // logmsg(message);
    // print message to logfile (time, pid, iteration # of number of iterations)
    // logmsg("testsim", "testsim: pid %d\n", getpid());
  }

  return 0;
}

// A Function to Deallocate Shared Memory
void deallocateSharedMemory(int shmid) {
  // Deallocate shared memory

}