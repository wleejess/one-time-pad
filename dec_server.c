// From exploration
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define TOTAL_VALID 27
char decryptedMsg[256];

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

int decrypt(char* message, char* key, char* decryptedMsg){
  // Same mapping array as keygen.
  const char valid_chars[TOTAL_VALID] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

  int msgChars[strlen(message)];
  int keyChars[strlen(key)];

  // Convert msg to char indices based on valid_chars.
  for(int i=0; i < strlen(message); i++) {
    for(int j=0; j < TOTAL_VALID; j++) {
      if (message[i] == valid_chars[j]) msgChars[i] = j;
    }
  }

  // Convert key to char indices based on valid_chars.
  for(int k=0; k < strlen(key); k++) {
    for(int l=0; l < TOTAL_VALID; l++) {
      if (key[k] == valid_chars[l]) keyChars[k] = l;
    }
  }

  // Subtract key from cipher text using modular arithmetic.
  for(int i=0; i < strlen(message); i++) {
    int diff = msgChars[i] - keyChars[i];
    printf("msgchar ind and keychar ind %d %d\n", msgChars[i], keyChars[i]);

    if (diff < 0) diff = diff + TOTAL_VALID;
    int val = diff % 27;
    printf("val for decrypt inde %d\n", val);
    message[i] = valid_chars[val];

    printf("msg char is %c\n", message[i]);
  }

  decryptedMsg[strlen(message)] = '\0';

  return 0;
}

int main(int argc, char *argv[]) {
  int connectionSocket;
  char buffer[256];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  if (argc < 2) {
    fprintf(stderr, "USAGE %s port\n", argv[0]);
    exit(1);
  }

  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("DEC_SERVER: ERROR opening socket");
  }

  setupAddressStruct(&serverAddress, atoi(argv[1]));

  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    error("DEC_SERVER: ERROR on binding");
  }
  
  listen(listenSocket, 5);

  while (1) {

    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);
    if (connectionSocket < 0) {
      error("DEC_SERVER: ERROR on accept");
    }

    pid_t childPid = fork();

    if (childPid < 0) {
      perror("fork failed");
      exit(1);
    }

    if (childPid == 0) {
      
      char id_check[4];
      memset(id_check, '\0', sizeof(id_check));
      int charsRead = recv(connectionSocket, id_check, sizeof(id_check) - 1, 0);
      if (charsRead < 0) error("DEC_SERVER: ERROR reading from socket");

      if (strcmp(id_check, "enc") == 0) {
          fprintf(stderr, "DEC_SERVER: Connection rejected from enc client.\n");
          close(connectionSocket);
          exit(1);
      }

      char key[256];
      char message[256];

      memset(key, '\0', sizeof(key));
      memset(message, '\0', sizeof(message));
      memset(decryptedMsg, '\0', sizeof(decryptedMsg));

      charsRead = recv(connectionSocket, key, sizeof(key) - 1, 0);
      if (charsRead < 0) error("DEC_SERVER: ERROR reading key from socket");

      charsRead = recv(connectionSocket, message, sizeof(message) - 1, 0);
      if (charsRead < 0) error("DEC_SERVER: ERROR reading msg from socket");

      if (strlen(key) < strlen(message)) {
        fprintf(stderr, "DEC_SERVER: Key too short!\n");
        exit(1);
      }

      printf("message before decrypt %s\n", message);
      decrypt(message, key, decryptedMsg);
      printf("message after decrypt %s\n", message);

      charsRead = send(connectionSocket, message, strlen(message), 0);
      if (charsRead < 0) error("DEC_SERVER: ERROR writing to socket");
    } else {
      close(connectionSocket);
    }
  }
  close(listenSocket);
}
