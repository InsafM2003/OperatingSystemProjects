// The MIT License (MIT)
// 
// Copyright (c) 2024 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>

#define WHITESPACE " \t\n"      
                                

#define MAX_COMMAND_SIZE 255    

#define MAX_NUM_ARGUMENTS 32     

int main(int argc, char * argv[])
{
  char * command_string = (char*) malloc( MAX_COMMAND_SIZE );
  char error_message[30] = "An error has occurred\n";
  FILE *batch_file = NULL;
  
  //Batch mode
  if(argc > 2)
  {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }
  if(argc == 2)
  {
    batch_file = fopen(argv[1], "r");
    if(batch_file == NULL)
    {
      write(STDERR_FILENO, error_message, strlen(error_message));
      exit(EXIT_FAILURE);
    }
  }
  
  while( 1 )
  {
    //Reading from batch file
    fgets(command_string, MAX_COMMAND_SIZE, batch_file);
    
    //Check for EOF
    if(feof(batch_file)) exit(0);
    
    //Interactive mode
    if( argc == 1)
    {
      printf("msh> ");
      while(!fgets(command_string, MAX_COMMAND_SIZE, stdin));
    }
    
  
    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;                                 

    char *argument_pointer;                                         
                                                           
    char *working_string  = strdup( command_string );                

    char *head_ptr = working_string;
    
    while ( ( (argument_pointer = strsep(&working_string, WHITESPACE ) ) != NULL) &&
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( argument_pointer, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
      else
      {
        token_count++;
      }
    }
    

    // exit and cd function
    if (token[0] == NULL) 
    {
      continue;
    }
    if(strcmp(token[0],"exit") == 0)
    {
      if( token[1] != 0)
      {
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
      exit(0);
    }
    if(strcmp(token[0],"cd") == 0)
    {
      if(chdir(token[1]) != 0)
      {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
      }
      continue;
    }
    
    //Forking
    pid_t pid = fork();
    
    if( pid == -1 )
    {
      write(STDERR_FILENO, error_message, strlen(error_message));
      exit( EXIT_FAILURE );
    }
    else if (pid == 0)
    {
      //Redirection
      for(int i=1; i<token_count; i++)
      {
        if(token[i] == NULL)
        {
          continue;
        }
        if( strcmp( token[i], ">" ) == 0)
        {
          int fd = open( token[i+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR );
          if( fd < 0 || token[i+2] != NULL)
          {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);                    
          }
          dup2( fd, 1 );
          close( fd );
            
          token[i] = NULL;
          token[i+1] = NULL;
         }
      }
      execvp(token[0],&token[0]);
      write(STDERR_FILENO, error_message, strlen(error_message));
      exit(EXIT_SUCCESS);
    }
    else
    {
      int status;
      waitpid( pid, &status, 0 );
    }  
  free(head_ptr);
  }
  return 0;
}

