// Blocks until a license is available
int getlicence(void);

// Increments the # of licenses available
int returnlicense(void);

// Performs any needed initialization of the license object
int initlicense(void);

// Adds n licenses to the number available
void addtolicenses(int n);

// Decrements the number of licenses by n
void removelicenses(int n);

/**
 * Write the specified message to the log file.
 * There's only 1 log file.
 * This function will treat the log file as a critical resource.
 * It will open the file to append the message, and close the file after appending the message.
 */
void logmsg(const char * msg);