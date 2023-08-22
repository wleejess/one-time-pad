// From exploration.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>      // ssize_t
#include <sys/socket.h>     // send(), recv()
#include <netdb.h>          // gethostbyname

// Create a socket and connect to sever specified in command arguments.
// Prompt user for input and sends that input as a msg to the server.
// Print the msg received from the server and exit the program.

// Error function for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up address structure
void setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname) {
  memset((char*) address, '\0', sizeof(*address));

  address->sin_family = AF_INET;                // Address should be network capable
  address->sin_port = htons(portNumber);        // Store the port number
  
  struct hostent* hostInfo = gethostbyname(hostname);
  if (hostInfo == NULL) {
    fprintf(stderr, "ENC_CLIENT: ERROR, no such host\n");
    exit(0);
  }

  // Copy the first IP address from DNS entry to sin_addr.s_addr.
  memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber;
  struct sockaddr_in serverAddress;
  char buffer[256];

  // Check usage & arguments
  if (argc < 4) {
    fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);
    exit(1);
  }

  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0) {
    error("ENC_CLIENT: ERROR opening socket");
  }

  // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server 
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("ENC_CLIENT: ERROR connecting");
  }

  const char* id_check = "enc\0";
  send(socketFD, id_check, strlen(id_check), 0);

  FILE *plaintextFile = fopen(argv[1], "r");
  if (!plaintextFile) error("ENC_CLIENT: ERROR opening plaintext file");

  FILE *keyFile = fopen(argv[2], "r");
  if (!keyFile) error("ENC_CLIENT: ERROR opening key file");

  char plaintext[256];
  memset(plaintext, '\0', sizeof(plaintext));
  fgets(plaintext, sizeof(plaintext), plaintextFile);
  plaintext[strcspn(plaintext, "\n")] = '\0';
  fclose(plaintextFile);

  char key[256];
  memset(key, '\0', sizeof(key));
  fgets(key, sizeof(key), keyFile);
  key[strlen(key)] = '\0';
  fclose(keyFile);

  if (strlen(key) < strlen(plaintext)) {
    fprintf(stderr, "ENC_CLIENT: ERROR - Key is too short to encrypt plaintext.\n");
    exit(1);
  }

  // Send msg to server and write to server.
  ssize_t charsWritten = send(socketFD, key, strlen(key), 0);
  if (charsWritten < 0) error("CLIENT: ENC_CLIENT: ERROR writing to socket");
  if (charsWritten < strlen(key)) error("CLIENT: WARNING - not all data written.");

  charsWritten = send(socketFD, plaintext, strlen(plaintext), 0);
  if (charsWritten < 0) error("CLIENT: ENC_CLIENT: ERROR writing plaintext to socket.");
  if (charsWritten < strlen(plaintext)) error("CLIENT: WARNING - not all data written.");
  

  // Get return message from server
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end.
  ssize_t charsRead = recv(socketFD, buffer, sizeof(buffer), 0);
  if (charsRead < 0) error("CLIENT: ENC_CLIENT: ERROR reading from socket");
  printf("%s\n", buffer);
  close(socketFD);
  return 0;
}

