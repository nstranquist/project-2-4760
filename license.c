#include "license.h"
#include "config.h"

// Note: All of these functions are critical sections

// Blocks until a license is available
int getlicense(void) {
  sleep(2);
  if(nlicenses->nlicenses <= 0)
    return 1;

  printf("licenses: %d\n", nlicenses->nlicenses);

  // decrement licenses
  // nlicenses->nlicenses = nlicenses->nlicenses - 1;

  // printf("next licenses: %d\n\n", nlicenses->nlicenses);

  return 0;
}

// Increments the # of licenses available
int returnlicense(void) {
  sleep(2);
  if(nlicenses->nlicenses >= nlicenses->nlicenses_max || nlicenses->nlicenses >= MAX_LICENSES) {
    printf("runsim: Warning: Max licenses reached\n");
    return 1;
  }
  nlicenses->nlicenses = nlicenses->nlicenses + 1;
  printf("returnlicense: %d licenses available\n", nlicenses->nlicenses);
  return 0;
}

// Performs any needed initialization of the license object
int initlicense(int max) {
  sleep(1);
  // set both to max initially, but only nlicenses will change up and down
  nlicenses->nlicenses = max;
  nlicenses->nlicenses_max = max;
  return 0;
}

// Adds n licenses to the number available
void addtolicenses(int n) {
  sleep(1);
  if(nlicenses->nlicenses + n > nlicenses->nlicenses_max) {
    printf("runsim: Warning: Max licenses reached. Cannot add more.\n");
    return;
  }
  if(n < 0) {
    printf("runsim: Warning: Cannot add negative licenses to total amount.\n");
    return;
  }

  printf("adding %d licenses to nlicenses\n", n);

  nlicenses->nlicenses = nlicenses->nlicenses + n;
}

// Decrements the number of licenses by n
void removelicenses(int n) {
  sleep(1);
  if(nlicenses->nlicenses - n < 0) {
    printf("runsim: Warning: removing %d licenses overflows total amount below 0. Keeping the amount at 0.\n", n);
    nlicenses->nlicenses = 0;
    return;
  }
  if(n < 0) {
    printf("runsim: Warning: Cannot remove negative licenses.\n");
    return;
  }

  nlicenses->nlicenses = nlicenses->nlicenses - n;

  printf("removing %d license, leaving %d remaining\n", n, nlicenses->nlicenses);
}

/**
 * Write the specified message to the log file.
 * There's only 1 log file.
 * This function will treat the log file as a critical resource.
 * It will open the file to append the message, and close the file after appending the message.
 */
void logmsg(const char * msg) {
  sleep(1);
  char *filename = "runsim.log";

  // Open the log file
  FILE * fp = fopen(filename, "a");

  if(fp == NULL) {
    printf("runsim: Error: Could not open log file %s for writing.\n", filename);
    return;
  }

  // Write the message to the log file
  fprintf(fp, "%s\n", msg);

  // Close the log file
  fclose(fp);
}