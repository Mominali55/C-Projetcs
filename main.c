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