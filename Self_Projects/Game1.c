#include<stdio.h>
#include<string.h>
#include <stdlib.h> // for rand() and srand()
#include <time.h>   // for time()

int game(char you[],char computer[]){
    if(strcmp(you,computer) == 0){
        return -1;
    }

    //Comparing stone
    if(strcmp(you,"stone") == 0 && strcmp(computer,"paper") == 0 ){
        printf("\n Computer chosed:---%s \n",computer);
        return 0; //lose
    }else if (strcmp(you,"stone") == 0 && strcmp(computer,"zissor") == 0){
        printf("\n Computer chosed:---%s",computer);
        return 1; //win
    }
    if(strcmp(you,"paper") == 0 && strcmp(computer,"stone") == 0){
        printf("\n Computer chosed:---%s",computer);
        return 1; //win
    }else if (strcmp(you,"paper") == 0 && strcmp(computer,"zissor") == 0){
        printf("\n Computer chosed:---%s",computer);
        return 0; //lose
    }
    if(strcmp(you,"zissor") == 0 && strcmp(computer,"stone") ==0){
        printf("\n Computer chosed:---%s",computer);
        return 1; //win
    }else if (strcmp(you,"zissor") == 0 && strcmp(computer,"paper") == 0){
        printf("\n Computer chosed:---%s",computer);
        return 0; //lose
    }

}

int main(){
    printf("\n\n\n\nHello welcome to the stone,paper,zissor gamen\n\n\n\n");
    
    //Defining the datstype
    char you[20];
    char computer[20];
    int result,n;

    //Random generater
    srand(time(NULL));
    n = rand()%100;

    if(n<33)
    {
        strcpy(computer,"stone");
    }else if(n > 33 && n < 66){
        strcpy(computer,"paper");
    }else{
        strcpy(computer,"zissor");
    }

    //Now for user
    printf("\nPlz enter stone,paper,zissor:");
    scanf("%s",you);
    result=game(you,computer);
    if(result == -1){
        printf("\n--Draw--\n");
    }else if(result == 1){
        printf("\n--You win!--\n");
    }else if(result == 0){
        printf("\n--You lose!--\n");
    }else{
        printf("\n--Error--\n");
    }
}