/**
 * Author: Nico Stranquist
 * Date: October 3, 2021
 * 
 *
 * Implement 'runsim' as follows:
 * 1. Check CLI arguments and output a usage message if argument is not appropriate
 * 2. Allocate shared memory and populate it with the number of licenses from Step 1
 * 3. Execute the main loop until stdin is end-of-file is reached on stdin
 *  a. Read a line from stdin of up to MAX_CANON characters (fgets)
 *  b. Request a license from the License object
 *  c. Fork a child that does 'docommand'. docommand will request a license from the license manager object.
 *      Notice that if the license is not available, the request function will go into wait state.
 *  d. Pass the input string from step (a) to docommand. The docommand function will execl the specified command
 *  e. The parent (runsim) checks to see if any of the children have finished (waitpid with WNOHANG option).
 *      It will `returnlicense` when that happens
 *  f. After encountering EOF on stdin, `wait` for all the remaining children to finish and then exit
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "config.h"
#include "license.h"

// NOTE: You are required to use fork, exec (or one of its variants) ,wait, and exit to manage multiple processes.

// You will need to set up shared memory in this project to allow the processes to communicate with each other.
// Please check the man pages for shmget, shmctl, shmat, and shmdt to work with shared memory.

// Function definitions
void docommand(char *cline);
int detachandremove(int shmid, void *shmaddr);
char * getTimeFormattedMessage(char *msg);

// Bakery functions
void critical_section();
void remainder_section();
int max(int *array, int size);
void process_i(const int i);

int shmid;
void *shmaddr;

// bakery variables, should be in shared memory
int choosing[BAKERY_SIZE]; // shm boolean array
int number[BAKERY_SIZE]; // shm integer array to hold turn number

static void myhandler(int signum) {
  if(signum == SIGINT) {
    // is ctrl-c interrupt
    printf("\nCtrl-C Interrupt Detected. Shutting down gracefully...\n");
  }
  else if(signum == SIGPROF) {
    // is timer interrupt
    printf("\nThe time for this program has expired. Shutting down gracefully...\n");
  }

  // free memory and exit
  nlicenses = (int *)shmat(shmid, NULL, 0);
  int result = detachandremove(shmid, nlicenses);
  printf("detachandremove result: %d", result);
  pid_t group_id = getpgrp();
  killpg(group_id, signum);

  if(result == -1) {
    fprintf(stderr, "runsim: Error: Failure to detach and remove memory\n");
  }

  // Print time to logfile before exit
  char *msg = getTimeFormattedMessage(" - Termination");

  logmsg(msg);

	exit(1);
}

static int setupinterrupt(void) {
  struct sigaction act;
  act.sa_handler = myhandler;
  act.sa_flags = 0;
  return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}

static int setupitimer(void) {
  struct itimerval value;
  value.it_interval.tv_sec = SLEEP_TIME;
  value.it_interval.tv_usec = 0;
  value.it_value = value.it_interval;
  return (setitimer(ITIMER_PROF, &value, NULL));
}

// Write a runsim program that runs up to n processes at a time. Start the runsim program by typing the following command
int main(int argc, char *argv[]) {
  // Validate CLI Arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <number-of-licenses>\n", argv[0]);
    return -1;
  }
  if(!atoi(argv[1])) {
    fprintf(stderr, "Usage: %s <number-of-licenses>, where n is an integer\n", argv[0]);
    return -1;
  }

  // parse argv[1] to get the number of licenses available at the same time
  int nlicensesInput = atoi(argv[1]);

  if(nlicensesInput < 0) {
    fprintf(stderr, "Usage: %s <number-of-licenses>, where n is an integer >= 0\n", argv[0]);
    return -1;
  }
  else if(nlicensesInput > MAX_LICENSES) {
    printf("runsim: Warning: Max Licenses at a time is %d\n", MAX_LICENSES);
  }

  // Set up timers and interrupt handler
  int err = setupitimer();
  if (err) {
    perror("runsim: Error: setupitimer");
    return -1;
  }
  err = setupinterrupt();
  if (err) {
    perror("runsim: Error: setupinterrupt");
    return -1;
  }

  signal(SIGINT, myhandler);

  printf("%d licenses specified\n", nlicensesInput);

  // allocate shared memory
  shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  printf("shmid: %d\n", shmid);
  if (shmid == -1) {
    perror("runsim: Error: Failed to create shared memory segment");
    return -1;
  }

  // attach shared memory
  nlicenses = (int *)shmat(shmid, NULL, 0);
  if (nlicenses == (void *) -1) {
    perror("runsim: Error: Failed to attach to shared memory");
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
      perror("runsim: Error: Failed to remove memory segment");
    return -1;
  }

  // initialize n licenses
  initlicense();

  // set nlicenses to nlicensesInput
  addtolicenses(nlicensesInput);

  // print nlicenses

  printf("new nlicenses value: %d\n", *nlicenses);

  printf("\n");

  char cline[MAX_CANON];

  // Main Loop until EOF reached
  while (fgets(cline, MAX_CANON, stdin) != NULL) {
    // fork a child
    pid_t pid = fork();
    if (pid == -1) {
      perror("runsim: Error: Failed to fork a child process");
      if (detachandremove(shmid, nlicenses) == -1)
        perror("runsim: Error: Failed to detach and remove shared memory segment");
      return -1;
    }

    // getlicense();

    nlicenses = (int *)shmat(shmid, NULL, 0);

    printf("cline: %s\n", cline);
    // printf("forked pid: %d\n", pid);

    // child's code if pid is 0
    if (pid == 0) {
      // request a license from the license manager object
      if(getlicense() == 1) {
        printf("Waiting for license to become available\n");
        // Q) Is this the correct wait()?
        wait(NULL);
        printf("Finished wait\n");
      }


      // Call docommand child
      docommand(cline);
    }
    // parent's code if pid > 0
    else {
      // parent waits inside loop for child to finish
      int status;
      pid_t wpid = waitpid(pid, &status, WNOHANG);
      if (wpid == -1) {
        perror("runsim: Error: Failed to wait for child");
        return -1;
      }
      else if(wpid == 0) {
        // child is still running
        printf("Child is still running\n");
      }
      else {
        printf("Child finished, wpid is %d . Returning license\n", wpid);
        // return license
        if(returnlicense() == 1) {
          printf("runsim: Error: Failed to return license\n");
        }
        printf("New licenses after return: %d", *nlicenses);
      }
    }
  }

  // Wait for all children to finish, after the main loop is complete
  while(wait(NULL) > 0) {
    printf("Waiting for all children to finish\n");
  }

  printf("All children supposedly finished\n");

  nlicenses = (int *)shmat(shmid, NULL, 0);
  if (nlicenses == (void *) -1) {
    perror("runsim: Error: Failed to attach to shared memory");
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
      perror("runsim: Error: Failed to remove memory segment");
    return -1;
  }


  if(detachandremove(shmid, nlicenses) == -1) {
    perror("runsim: Error: Failed to detach and remove shared memory segment");
    return -1;
  }

  // log message before final termination
  
  char *msg = getTimeFormattedMessage(" - Termination");
  logmsg(msg);

  // detach shared memory
  // int detachResult = shmdt(shm);
  // printf("detachResult: %d\n", detachResult);
  // if (detachResult == -1) {
  //   perror("shmdt");
  //   return -1;
  // }
  // // remove shared memory
  // int removeResult = shmctl(shmid, IPC_RMID, NULL);
  // printf("removeResult: %d\n", removeResult);
  // if (removeResult == -1) {
  //   perror("shmctl");
  //   return -1;
  // }
  // Check for the correct number of command-line arguments and output a usage message if incorrect.
  // get CLI arguments and validate

  // Allocate shared memory and populate it with # licenses from Step 1

  // run main loop until EOF found
  // 1. Read a line from stdin of up to MAX_CANON characters (fgets)
  // 2. Request a license from the License object
  // 3. Fork a child that does 'docommand'. docommand will request a license from the license manager object. If the license is not available, the request function will go into wait state.
  // 4. Pass the input string from step (1) to docommand. The docommand function will execl the specified command
  // 5. Check to see if any other children have finished (waitpid with WNOHANG option). It will returnlicense when that happens
  // 6. After encountering EOF on stdin, wait for all the remaining children to finish and then exit

  return 0;
}

void docommand(char *cline) {
  printf("received in docammand: %s\n", cline);

  // Fork to grand-child
  pid_t grandchild_id = fork();

  printf("forked grandchild: %d\n", grandchild_id);

  // check if license available as well

  // while(getlicense() == 1) {
  //   printf("waiting on license\n");
  //   wait(NULL);
  // }
  if(getlicense() == 1) {
    printf("docommand: Waiting for license to become available\n");
    wait(NULL);
  }
  
  printf("Checking grandchild id: %d\n", grandchild_id);
  if (grandchild_id == -1) {
    perror("runsim: Error: Failed to fork grand-child process");
    return;
  }
  else if (grandchild_id == 0) {
    // grand-child
    // printf("grandchild: %d\n", grandchild_id);
    // get first word from cline
    char *command = strtok(cline, " ");
    // get the rest of the words in the line
    char *arg2 = strtok(NULL, " ");
    char *arg3 = strtok(NULL, " ");
    // printf("command: %s\n", command);
    // printf("args: %s %s\n", arg2, arg3);
    // execl the command
    execl(command, command, arg2, arg3, (char *) NULL);
    perror("runsim: Error: Failed to execl");
    // Q) clean up memory?
    // wait(NULL);
  }
  else {
    // parent
    // printf("parent: %d\n", getpid());
    // printf("grandchild: %d\n", grandchild_id);
    // printf("cline: %s\n", cline);
    int grandchild_status;
    // Q) do we want no hang option here? do we want to wait for grandchild_id or any id (-1)?
    waitpid(grandchild_id, &grandchild_status, WNOHANG);
    printf("Grand child finished, result: %d\n", WEXITSTATUS(grandchild_status));
    returnlicense();
  }

  exit(0);
  // 1. Fork a child (a grandchild of the original).
    // This grandchild calls makeargv on cline and calls execvp on the resulting argument array.

  // 2. Wait for this child and then return the license to the license object.

  // 3. Exit (exit()?)
}

// From textbook
int detachandremove(int shmid, void *shmaddr) {
  printf("cleaning up id %d\n", shmid);

  int error = 0;

  if (shmdt(shmaddr) == -1) {
    printf("runsim: Error: Can't detach memory\n");
    error = errno;
  }

  if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error) {
    printf("runsim: Error: Can't remove memory\n");
    error = errno;
  }

  if (!error)
    return 0;

  errno = error;

  return -1;
}

char * getTimeFormattedMessage(char *msg) {
  time_t tm = time(NULL);
  time(&tm);
  struct tm *tp = localtime(&tm);

  char time_str [9];
  sprintf(time_str, "%.2d:%.2d:%.2d", tp->tm_hour, tp->tm_min, tp->tm_sec);

  int msg_length = strlen(msg) + strlen(time_str);

  char *formatted_msg = (char*)malloc(msg_length * sizeof(char));

  sprintf(formatted_msg, "%s%s", time_str, msg);

  return formatted_msg;
}

// Bakery Algorithm
// - easier algorithm to understand and implement
// - each process has a unique id (bitmap array)
// - process id is assigned in a completely ordered manner
// - unsigned character has 8 bits, can handle 7 processes
// Can see exercise 5.10, page 251

// Variables for bakery are declared at top of file

void process_i(const int i) {
  do {
    choosing[i] = 1; // 1 is true
    number[i] = 1 + max(number, BAKERY_SIZE); // get highest number within array and add 1, to define a new turn
    choosing[i] = 0; // 0 is false

    for(int j = 0; j < BAKERY_SIZE; j++) {
      while(choosing[j]); // wait while someone else is choosing
      while((number[j]) && (number[j], j) < (number[i], j)); // (a,b) < (c,d) === (a < c) || ((a == c) && (b < d))
    }

    // enter critical section
    critical_section();

    // exit critical section
    number[i] = 0; // resets turn number

    remainder_section();
  } while(1);
}

// gets the highest number in an integer array
int max(int *array, int size) {
  int max = array[0];
  for (int i = 1; i < size; i++) {
    if (array[i] > max) {
      max = array[i];
    }
  }
  return max;
}

void critical_section() {
  printf("In critical section!\n");
}

void remainder_section() {
  printf("In remainder section.\n");
}