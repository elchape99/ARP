#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/*This function do an exec in child process*/

int spawn(const char * program, char ** arg_list) {
 
  pid_t child_pid = fork();
  if (child_pid != 0)
    //main process
    return child_pid;

   else {
    //child process
    execvp (program, arg_list);
    perror("exec failed");
    return 1;
 }
}

int main() {
  char * arg_list[] = { "knsole", "-e", "./first", NULL };
  spawn("konsole", arg_list);
  char * arg_list1[] = {"konsole", "-e", "./second", NULL};
  spawn ("konsole", arg_list1);
  printf ("Main program exiting...\n");
  return 0;
}
