#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()
#include <ctype.h>

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname(hostname); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;

  if (argc < 3) { 
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); 
    exit(0); 
  } 

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  
  }
  char function = 'D';
  charsWritten = send(socketFD, &function, 1,0);
  if (charsWritten < 0){
      error("Clinet writing on the socket");
    }
 
FILE *keycontents=fopen(argv[2], "r");
if (keycontents == NULL){
perror( "Error opening key file\n");
return(1);
}

FILE *plaintext = fopen(argv[1],"r");
if (plaintext == NULL){
perror("Error opening plain textfile\n");
return(1);
}

/*Generally used to reead the file contents from the end to the start since u need the size of the file 
and the u have to start from the beginning using seek_set otherwise no shit is read
*/
fseek(plaintext, 0L, SEEK_END);
int plaintextsize = ftell(plaintext);
fseek(plaintext, 0L, SEEK_SET);\
char plaintextSizestr[4];
sprintf(plaintextSizestr, "%03d", plaintextsize);

 fseek(keycontents, 0L, SEEK_END);
 int keysize = ftell(keycontents);
 fseek(keycontents, 0L, SEEK_SET);
 char keySizestr[6];
 sprintf(keySizestr, "%05d", keysize);




  // creation of buffers and placing them in arrays and placing them to enable movement of data via the created processes.
  char plaintextchars[1024];
  char realkey[1024];
  char plaitextlength[256];
  // sprintf(plaitextlength,"%d" , strlen(plaintextchars));
  // plaintextsize =strlen(plaintextchars);
  
  fgets(plaintextchars, sizeof(plaintextchars), plaintext);
  fgets(realkey, sizeof(realkey), keycontents);
  realkey[sizeof(realkey)-1] = '\0';
  for(int i = 0; i < plaintextchars[i] != '\0';i++){
  if (!isalpha(plaintextchars[i]) && plaintextchars[i] != ' '){
      fprintf(stderr,"wrong characters in plaintextfile\n");
      exit(1);
    }
  }
for(int i=0; i < realkey[i] !='\0' ;i++){
if (!isalpha(realkey[i]) && realkey[i] != ' '){
      fprintf(stderr,"wrong chars in file\n");
      exit(1);
    }
  }
if (strlen(realkey) < strlen(plaintextchars)){
      fprintf(stderr,"char is too short.\n");
      exit(1);
    }
  //printf("Clinet sending plain text chars: \"%s\"\n", plaintextSizestr);
  int lengthsent=0;
  while(lengthsent < strlen(plaintextSizestr)){
  charsWritten = send(socketFD, plaintextSizestr + lengthsent, strlen(plaintextSizestr)-lengthsent, 0); 
   //charsWrittens= send(socketFD, realkey, strlen(realkey), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  lengthsent += charsWritten;
}
  if (charsWritten < strlen(plaintextSizestr)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }



 int TotalSent = 0;
 //printf("Client sending plain text  chars: \"%s\"\n", plaintextchars);
while(TotalSent < plaintextsize){
  charsWritten = send(socketFD,plaintextchars +TotalSent,plaintextsize- TotalSent,0);
  if (charsWritten < 0){
    error("CLIENT: Error writing to socket");
  }
  TotalSent += charsWritten;
}
if (TotalSent < plaintextsize){
  printf("CLIENT: WARNING: Not all data writtent to the socket!\n"); 
} 

//printf("Clinet sending plain text chars: \"%s\"\n", plaintextchars);
  charsWritten = send(socketFD, keySizestr, strlen(keySizestr), 0); 
   //charsWrittens= send(socketFD, realkey, strlen(realkey), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(keySizestr)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }



int totalSent = 0;
while(totalSent < keysize){
  charsWritten = send(socketFD,realkey +totalSent,keysize- totalSent,0);
  if (charsWritten < 0){
    error("CLIENT: Error writing to socket");
  }
  totalSent += charsWritten;
}
if (totalSent < keysize){
  printf("CLIENT: WARNING: Not all data writtent to the socket!\n"); 
} 
//printf("Client keys sent: \"%s\"\n", realkey);
fclose(plaintext);
fclose(keycontents);

  char ciphertext[1024];
  charsRead = recv(socketFD, ciphertext,  sizeof(ciphertext), 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  ciphertext[charsRead] = '\0';
  printf(" Secretly encoded message: %s\n", ciphertext);

  // Close the socket
  close(socketFD); 
  return 0;
}
