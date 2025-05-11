#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 51234

//function that logs messages from likesServer and has a timestamp
void loggerPrimary(char *msg, int serverID){
  FILE *file = fopen("/tmp/primaryLog.log", "a");
  if(file == NULL){
    perror("error\n");
    exit(1);
  }
  time_t curr_time;
  struct tm *local;
  time(&curr_time);
  local = localtime(&curr_time);
  fprintf(file, "%s %s\n", asctime(local), msg);
  fclose(file);
}//end of logger

//validates if likes, ID, and name are in format/within bounds
int validData(char *buffer){
  
  int serverID, likes;

  if(sscanf(buffer, "LikesServer%d %d", &serverID, &likes) == 2){
    if(serverID >= 0 && serverID < 10 && likes >= 0 && likes <=42){
      return 1;
    }
  }
  return 0;
}//end of validData

int main(){
  int server, new_sock, totalLikes = 0;
  struct sockaddr_in addy;
  int addyLen = sizeof(addy);
  char buffer[1024] = {0};
  int client_sockets[10] = {0};

  //socket file descriptor
  if((server = socket(AF_INET, SOCK_STREAM, 0)) == 0){
      perror("Socket failed");
      exit(EXIT_FAILURE);
  }

  //configures server address
  addy.sin_family = AF_INET;
  addy.sin_addr.s_addr = INADDR_ANY;
  addy.sin_port = htons(PORT);

  //bind socket to port
  if(bind(server, (struct sockaddr *)&addy, sizeof(addy)) < 0){
    perror("bind failed\n");
    exit(EXIT_FAILURE);
  }
  
  //listen for connections
  if(listen(server, 10)<0){
    perror("Listen failed\n");
    exit(EXIT_FAILURE);
  }
  
  //now for never ending primaryLikesServer to run forever
  while(1){

    /*a select implementation that makes 
     *sure it gets every client when available
     *this implementation is a set with fixed buffers
     */
    fd_set read_fds;
    int max_sd = server;
    FD_ZERO(&read_fds);
    FD_SET(server, &read_fds);

    //adds the child processes into the set
    for(int i = 0; i < 10; i++){
      int sd = client_sockets[i];
      if(sd > 0) {
	FD_SET(sd, &read_fds);
      }
      if(sd > max_sd){
	max_sd = sd;
      }
    }
    //uses select to wait for any activity from the sockets
    if(select(max_sd + 1, &read_fds, NULL, NULL, NULL)<0){
      perror("select failed");
      exit(EXIT_FAILURE);
    }
    //checks for any new connection requests after the first
    if(FD_ISSET(server, &read_fds)){
      if((new_sock = accept(server, (struct sockaddr *)&addy, (socklen_t *)&addyLen))<0){
	perror("accept failed\n");
	exit(EXIT_FAILURE);
      }
      //now adds the new connection to an array of client sockets
      for(int i = 0; i < 10; i++){
	if(client_sockets[i] == 0){
	  client_sockets[i] = new_sock;
	  break;
	}
      }
    }
    //checks if any client sent data
    for(int i = 0; i < 10; i++){
      int sd = client_sockets[i];
      
      if(FD_ISSET(sd, &read_fds)){
	ssize_t valread = read(sd, buffer, 1023);
	if(valread <= 0){
	  //closes and removed from set
	  close(sd);
	  client_sockets[i] = 0;
	}else{
	  buffer[valread] = '\0';
	  printf("%s\n", buffer);
	}
	//validate data first
	if(validData(buffer)){
	//extracts the likes from buffer and update total likes
	int serverID, likes = 0;
	sscanf(buffer, "LikesServer%d %d", &serverID, &likes);
	totalLikes += likes;
	printf("Total likes: %d\n", totalLikes);
	
	//now log data and transaction
	char logMsg[500];
	snprintf(logMsg, sizeof(logMsg), "LikesServer%d sent %d likes!\nTotal likes: %d",serverID, likes, totalLikes);
	loggerPrimary(logMsg, serverID);
	
    //send response
	send(sd, "Likes received!", 15, totalLikes); //15 is size of msg, 0 for flags
	}//valid data buffer end
      } //FD_ISSET end
    }//for loop for array end
  }//while loop end
  
  return 0;
}
