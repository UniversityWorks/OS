#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>


void error(const char* message);



int main(int argc, char **argv)
{
  if(argc < 3) error("there are no arguments in stdin stream.\n");

  int pos_only_flag = ((strcmp("--positive-only", argv[1]) == 0) ? 1 : 0);
  
  const char *res_message = ((pos_only_flag) ? "only positive numbers" : "all numbers");
  printf("%s: ", res_message);
  for(int i = 1 + pos_only_flag;i< argc; i++)
  {
    if(pos_only_flag && *argv[i] == '-') continue;
    
    printf("%s ", argv[i]);
  }
  printf("\n"); 


  return 0;
}



void error(const char* message)
{
  printf("stdin error: %s",message);
  exit(1);
}

