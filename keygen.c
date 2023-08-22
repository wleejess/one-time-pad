// Program creates a key file of specified length.
// Characters in the file will be any of the 27 allowed characters using standard Unix randomization methods.
// Last character keygen outputs should be a newline. Error text will output to stderr.

#include <stdlib.h>
#include <stdio.h>

const char valid_chars[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

int main (int argc, char* argv[]) {
  int i;
  int length = atoi(argv[1]);
  char key[length + 1];

  for (i=0; i < length; i++) {
    int char_ind = rand() % 27;
    key[i] = valid_chars[char_ind];
  }

  key[length] = '\0';

  printf("%s\n", key);

  return 0;
}
