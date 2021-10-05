#include <stdio.h>
#include <stdlib.h>

#define BAKERY_SIZE 99 // is this big enough? each child may request multiple critical_section() actions

#define SLEEP_TIME 100

#define MAX_LICENSES 20

#define MAX_CANON 150

// Previous shared memory for nlicenses idea
// int *nlicenses;

// Next idea for shared memory for nlicenses, to include 1. Bakery Logic, 2. Max Licenses
struct License {
  int nlicenses; // used the same as before
  int nlicenses_max; // defaults to MAX_LICENSES but can be lowered to 'n' supplied in CLI argument

  // bakery information
  int choosing[BAKERY_SIZE];
  int number[BAKERY_SIZE];
};

struct License *nlicenses;