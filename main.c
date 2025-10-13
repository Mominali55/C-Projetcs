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