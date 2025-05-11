#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

void logger(char* msg){
  FILE *file = fopen("/tmp/ParentProcessStatus.log", "a");
  if(file == NULL){
    perror("Error\n");
    exit(EXIT_FAILURE);
  }
  time_t currTime;
  time(&currTime);
  fprintf(file, "%s: %s\n", ctime(&currTime), msg);
  fclose(file);

}//end of logger

int main(){

  //creating 10 childs
  for(int i = 0; i < 10; i++){
    pid_t pid = fork(); //forks a process but nothing loaded into it
    if(pid < 0){
      perror("Something not good");
      return 1;
    }
    else if (pid == 0){
      char LikeServer[2];
      sprintf(LikeServer, "%d", i);
      //below replaces a process with a LikeServer
      execl("./likes", "LikesServer", LikeServer, NULL);
      exit(0);
    }
    else{
      //parent code
      char message[100];
      sprintf(message, "LikesServer%d started", i);
      logger(message);
    }
    sleep(1); //wait one second to start next process; not needed but...
  }

  //waits until all childs terminate then can exit
  for(int i = 0; i < 10; i++){
    int status;
    wait(&status);

    if(WIFEXITED(status)){
      char end[100];
      sprintf(end, "LikesServer%d terminated", i);
      logger(end);
    }
  }
  
  logger("Parent process terminated");
  return 0;
}
