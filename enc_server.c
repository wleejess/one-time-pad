#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define TOTAL_VALID 27
#define TOTAL_INVALID 33

void error(const char *msg) {
  perror(msg);
  exit(1);
}

void setupAddressStruct(struct sockaddr_in* address, int portNumber) {
  memset((char*) address, '\0', sizeof(*address));
  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);
  address->sin_addr.s_addr = INADDR_ANY;
}

int encrypt(char* message, char* key) {
  // Same mapping array as keygen.
  const char valid_chars[TOTAL_VALID] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  const char invalid_chars[TOTAL_INVALID] = "abcedfghijklmnopqrstuvwxyz!#$*%?@";

  int msgChars[strlen(message)];
  int keyChars[strlen(key)];

  // Check if there are invalid characters.
  for(int i=0; i < strlen(message); i++) {
    for(int j=0; j < TOTAL_INVALID; j++) {
      if (message[i] == invalid_chars[j]) {
        error("ENC_SERVER: Invalid character found. No message will be encrypted.");
        return 1;
      }
    }
  }
 
  // Convert msg to char indices based on valid_chars.
  for(int i=0; i < strlen(message); i++) {
    for(int j=0; j < TOTAL_VALID; j++) {
      if (message[i] == valid_chars[j]) {
        msgChars[i] = j;
      }
    }
  }

  // Convert key to char indices based on valid_chars.
  for(int k=0; k < strlen(key); k++) {
    for(int l=0; l < TOTAL_VALID; l++) {
      if (key[k] == valid_chars[l]) keyChars[k] = l;
    }
  }

  // Perform modulo 27 addition of msg and key, store in encrypted array.
  for(int i=0; i < strlen(message); i++) {
    int sum = msgChars[i] + keyChars[i];
    int val = sum % 27;
    message[i] = valid_chars[val];
  }

  return 0;
}

int main(int argc, char *argv[]) {
  int connectionSocket;
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  if (argc < 2) {
    fprintf(stderr, "USAGE %s port\n", argv[0]);
    exit(1);
  }

  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ENC_SERVER: ERROR opening socket");
  }

  setupAddressStruct(&serverAddress, atoi(argv[1]));

  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    error("ENC_SERVER: ERROR on binding");
  }

  listen(listenSocket, 5);

  while (1) {
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
    if (connectionSocket < 0) {
      error("ENC_SERVER: ERROR on accept");
    }

    pid_t childPid = fork();

    if (childPid < 0) {
      perror("fork failed");
      exit(1);
    }

    if (childPid == 0) {
      // Check if connection is from dec_client. Should only take if from enc_client*
      char id_check[4];
      memset(id_check, '\0', sizeof(id_check));
      ssize_t charsRead = recv(connectionSocket, id_check, sizeof(id_check) - 1, 0);
      if (charsRead < 0) error("ENC_SERVER: ERROR reading from socket");

      if (strcmp(id_check, "dec") == 0) {
        fprintf(stderr, "ENC_SERVER: Connection from dec_client rejected");
        close(connectionSocket);
        exit(1);
      } 

      char key[2000];             
      char message[2000];

      while (1) {
        memset(key, '\0', sizeof(key));
        memset(message, '\0', sizeof(message));

        charsRead = recv(connectionSocket, key, sizeof(key) - 1, 0);
        if (charsRead < 0) error("ENC_SERVER: ERROR reading from socket");
        if (charsRead < strlen(key)) error("ENC_SERVER: WARNING not all characters read.");

        charsRead = recv(connectionSocket, message, sizeof(message) - 1, 0);
        if (charsRead < 0) error("ENC_SERVER: ERROR reading from socket");
        if (charsRead < strlen(message)) error("ENC_SERVER: WARNING not all characters read.");

        if (strlen(key) < strlen(message)) {
          fprintf(stderr, "Key too short!\n");
          exit(1);
        }

        int permittedMsg = encrypt(message, key);
        if (permittedMsg == 1) {
          close(connectionSocket);
          exit(0);
        }

        ssize_t charsWritten = send(connectionSocket, message, strlen(message), 0);
        if (charsWritten < 0) error("ENC_SERVER: WARNING not all characters written");
      }
      close(connectionSocket);
      exit(0);
    } else {
      close(connectionSocket);
    }
  }
  close(listenSocket);
  return 0;
}
