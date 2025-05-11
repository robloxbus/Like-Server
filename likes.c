#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <signal.h> //for 5 minute terminate


//a random port number
#define PORT 51234
//time for termination (may change for testing if you don't want to sit there)
#define TIMEOUT 300

//function to terminate after 5 minutes
void killLikes(int signaler){
  if(signaler == SIGALRM){
    printf("Server is terminating after 5 minutes of run time!\n");
    exit(0);
  }
}//end killLikes

//function to write to log
void loggerLikes(char *msg,int serverID, int likes){
  FILE *file;
  char likeFileName[100];
  snprintf(likeFileName, sizeof(likeFileName), "/tmp/LikesServer%d.log", serverID);
  file = fopen(likeFileName, "a");
  if(file == NULL){
    perror("error\n");
    exit(1);
  }
  
  time_t start;
  time(&start);
  fprintf(file, "%s: %s %d\n ", ctime(&start),msg,likes);
  fclose(file);
  
} //end of loggerLikes

//client side
int main(int argc, char *argv[]){
  
  //signal and alarm to terminate after 5 minutes
  signal(SIGALRM, killLikes);
  alarm(TIMEOUT);
  if(argc < 2){
    fprintf(stderr, "Format as %s serverID\n", argv[0]);
    return 1;
  }
  
  //format should be 'LikesServer ServerID'
  int serverID = atoi(argv[1]);
  char logMsg[100];
  snprintf(logMsg, sizeof(logMsg), "LikesServer%d started!",serverID);
  loggerLikes(logMsg, serverID,0); //writes to log for start of client

  int client = 0;
  struct sockaddr_in serv_addr;
  if((client = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("Error making socket\n");
    return -1;
  }
  //stuff for building sockets
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  //more error checking
  if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<= 0){
    printf("\nInvalid Address\n");
    return -1;
  }
  if((connect(client, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0){
    printf("\nConnection failed!\n");
    return -1;
  }

  while(1){
    int likes = 0;
    int randomTime = 0;
    //generate some random amount of likes
    srand(time(0) + serverID); //just for more randomness
    likes = rand() % 43;
    randomTime = (rand() % 5) + 1; //after 1-5 secs, more likes are sent
    
    //send them to the primaryLikesServer
    char message[100];
    snprintf(message, sizeof(message), "LikesServer%d %d",serverID,likes);
    send(client, message, strlen(message), 0);
    printf("LikesServer%d sent likes: %d\n",serverID, likes);
    loggerLikes("Likes sent: ", serverID, likes); //logs activity
    //receive response
    char buffer[500]={0};
    ssize_t valread = read(client, buffer, 1023); //1023 since 1024 is null terminator
    if(valread > 0){
      buffer[valread] = '\0';
      printf("Received response from primary: %s\n", buffer); //reads return message from primary
      char response[1000];
      snprintf(response, sizeof(response), "Response from primary: %s\n",buffer);
      loggerLikes(response, serverID, likes); //receives response including last likes amount sent
    }else{
      perror("Error reading from primary");
    }
    sleep(randomTime);
  }//end of while
  loggerLikes("LikesServer%d is terminated.", serverID, 0); //occurs when user ctrl+C, otherwise does not occur when timed out
  return 0;
}//end of main
