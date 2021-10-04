/**
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

#include <stdio.h>
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

// NOTE: You are required to use fork, exec (or one of its variants) ,wait, and exit to manage multiple processes.

// You will need to set up shared memory in this project to allow the processes to communicate with each other.
// Please check the man pages for shmget, shmctl, shmat, and shmdt to work with shared memory.

// Function definitions
void docommand(char *cline);
int detachandremove(int shmid, void *shmaddr);

int shmid;
int nLicenses;

static void myhandler(int signum) {
  if(signum == SIGINT) {
    // is ctrl-c interrupt
    printf("\nCtrl-C Interrupt Detected. Shutting down gracefully...\n");
  }
  else if(signum == SIGPROF) {
    // is timer interrupt
    printf("\nThe time for this program has expired. Shutting down gracefully...\n");
  }
  // char aster = "handler called";
  // int errsave;
  // errsave = errno;
  // write(STDERR_FILENO, &aster, 1);
  // errno = errsave;

  // free memory and exit
  pid_t group_id = getpgrp();
  // shmctl(shmid, IPC_RMID, NULL);
	// shmdt(nLicenses);
  killpg(group_id, signum);

  void *p;
  int result = detachandremove(shmid, p);
  printf("detachandremove result: %d", result);

  // Print time to logfile before exit
  // Get formatted time
  time_t tm = time(NULL);
  time(&tm);
  struct tm *tp = localtime(&tm);

  char time_str [9];
  sprintf(time_str, "%.2d:%.2d:%.2d", tp->tm_hour, tp->tm_min, tp->tm_sec);
  printf("Time: %s\n", time_str);

  // write to file
  char filename[] = "runsim.log";

  // write time_str to filename
  FILE *fp = fopen(filename, "a");
  fprintf(fp, "%s\n", time_str);
  fclose(fp);

  // exit
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
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <number-of-licenses>\n", argv[0]);
    return -1;
  }
  // Validate that argv[1] is a number and is greater than 0 and less than MAX_LICENSES
  if(!atoi(argv[1])) {
    fprintf(stderr, "Usage: %s <number-of-licenses>, where n is an integer\n", argv[0]);
    return -1;
  }

  // parse argv[1] to get the number of licenses
  int nlicenses = atoi(argv[1]);

  if(nlicenses < 0) {
    fprintf(stderr, "Usage: %s <number-of-licenses>, where n is an integer >= 0\n", argv[0]);
    return -1;
  }
  else if(nlicenses > MAX_LICENSES) {
    printf("Warning: Max Licenses at a time is %d\n", MAX_LICENSES);
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

  printf("%d licenses requested\n", nlicenses);

  // allocate shared memory
  int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  printf("shmid: %d\n", shmid);
  if (shmid == -1) {
    perror("shmget");
    return -1;
  }
  // attach shared memory
  int *shm = shmat(shmid, NULL, 0);
  if (shm == (void *) -1) {
    perror("shmat");
    return -1;
  }
  // populate shared memory
  *shm = nlicenses;

  char cline[MAX_CANON];

  // Main Loop
  while (fgets(cline, MAX_CANON, stdin) != NULL) {
    // fork a child
    pid_t pid = fork();
    if (pid == -1) {
      perror("fork");
      return -1;
    }

    printf("cline: %s\n", cline);
    printf("forked pid: %d\n", pid);

    if (pid == -1) {
      perror("fork");
      return -1;
    }
    if (pid == 0) {
      // Call docommand child
      docommand(cline);
      exit(0);
    }

    // parent
    int status;
    pid_t wpid = waitpid(-1, &status, WNOHANG);
    if (wpid == -1) {
      perror("waitpid");
      return -1;
    }
    if (wpid == 0) {
      printf("no child finished\n");
      // no child finished
      continue;
    }
    // a child finished
    // return license
    int *shm = shmat(shmid, NULL, 0);
    if (shm == (void *) -1) {
      perror("shmat");
      return -1;
    }
    *shm = *shm + 1;
  }

  // wait for all children to finish
  int status;
  pid_t wpid = waitpid(-1, &status, 0);
  printf("all children finished. wpid %d\n", wpid);
  if (wpid == -1) {
    perror("waitpid");
    return -1;
  }
  // detach shared memory
  int detachResult = shmdt(shm);
  printf("detachResult: %d\n", detachResult);
  if (detachResult == -1) {
    perror("shmdt");
    return -1;
  }
  // remove shared memory
  int removeResult = shmctl(shmid, IPC_RMID, NULL);
  printf("removeResult: %d\n", removeResult);
  if (removeResult == -1) {
    perror("shmctl");
    return -1;
  }
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



  // get first word from cline
  char *command = strtok(cline, " ");
  // get the rest of the words in the line
  char *arg2 = strtok(NULL, " ");
  char *arg3 = strtok(NULL, " ");
  printf("command: %s\n", command);
  printf("args: %s %s\n", arg2, arg3);
  // execl the command
  execl(command, command, arg2, arg3, (char *) NULL);
  perror("execl");
  exit(1);

  // 1. Fork a child (a grandchild of the original).
    // This grandchild calls makeargv on cline and calls execvp on the resulting argument array.

  // 2. Wait for this child and then return the license to the license object.

  // 3. Exit (exit()?)
}

// From textbook
int detachandremove(int shmid, void *shmaddr) {
   int error = 0;

   if (shmdt(shmaddr) == -1)
      error = errno;

   if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !error)
      error = errno;

   if (!error)
      return 0;

   errno = error;

   return -1;
}

