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
#include "config.h"

// Function definitions
void docommand(char *cline);


// Write a runsim program that runs up to n processes at a time. Start the runsim program by typing the following command
int main(int argc, char *argv[]) {
  // Check for the correct number of command-line arguments and output a usage message if incorrect.
  // get CLI arguments and validate

  // Allocate shared memory and populate it with # licenses from Step 1

  // run main loop until EOF found
  // 1. Read a line from stdin of up to MAX_CANON characters (fgets)
  // 2. Request a license from the License object
  // 3. Fork a child that does 'docommand'. docommand will request a license from the license manager object.
  // 4. Check to see if any other children have finished (waitpid with WNOHANG option).

  return 0;
}

void docommand(char *cline) {
  // 1. Fork a child (a grandchild of the original).
    // This grandchild calls makeargv on cline and calls execvp on the resulting argument array.

  // 2. Wait for this child and then return the license to the license object.

  // 3. Exit
  return;
}