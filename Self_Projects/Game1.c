#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<string.h>

int game(char you)
{
    char computer[]={'p','r','z'};
    if(strcmp(you,computer[1]) == 0 || strcmp(you,computer[2]) == 0 || strcmp(you,computer[3]) == 0)
        return 1;
}

int main(){
    char you='z';
    int result = game(you);
    printf("Draw %d",result); 
}