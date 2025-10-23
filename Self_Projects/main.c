//Creating a Shell in C
#include<stdio.h>
#include<stdlib.h>


int main(int argc,char **argv)
{
    //Loading configuration files if any

    //Running command loop
    ls_loop();

    //Performing any shutdown/cleanup

    //Using EXIT_SUCCESS is same as return 0;
    return EXIT_SUCCESS;
}

void lsh_loop(void)
{
    char *line; //to store the line read from stdin
    char **args; //to store the command and its arguments
    int status; //to check whether to continue the loop or not

    do{
        printf("> ");
        line = lsh_read_line();//reading a line from stdin
        args= lsh_split_line(line);//splitting the line into command and arguments
        status = lsh_execute(args);//executing the command

        free(line);
        free(args);
    } while (status);
}

#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
    int bufsize =LSH_RL_BUFSIZE;
    int position =0;
    char *buffer =malloc(sizeof(char)*bufsize);
    int c;

    //If the memory allocation fails
    if(!buffer) // If buffer is NULL
    {
        //fprintf is used to print to a file stream, here stderr
        fprintf(stderr,"lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        //Read a character
        c=getchar();
        //if we hit EOF, replace it with a null character and return
        if(c == EOF || c == '\n'){
            buffer[position] ='\0';
            return buffer;
        }else{
            buffer[position]=c;
        }
        position++;

        //if we excced the buffer, reallocate
        if(position >=bufsize){
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer,bufsize);
            if(!buffer){
                fprintf(stderr,"lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }

        }
    }
    
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

char **lsh_split_line(char *line)
{
    int bufsize =LSH_TOK_BUFSIZE,position=0; //How Two arg?
    char **tokens =malloc(bufsize *sizeof(char*));
    char *token;

    if(tokens == NULL)
    {
        fprintf(stderr,"LSH:Allocation failed\n");
        exit(EXIT_FAILURE);
    }

    //strtok() actually does is return pointers to within the string you give it, and place \0 bytes at the end of each token
    token = strtok(line,LSH_TOK_DELIM); //why use LSH...
    while (token != NULL) 
    {
        tokens[position]=token;
        position++;

        if (position >= bufsize){
            bufsize +=  LSH_TOK_BUFSIZE;
            tokens = realloc(tokens,bufsize*sizeof(char*));
            if(tokens ==NULL){
                fprintf(stderr,"LSH:Allocation failed\n");
                exit(EXIT_FAILURE);
            }
            
        }  
        token =strtok(NULL,LSH_TOK_DELIM);
    }
    tokens[position]=NULL;
    return tokens;   
}