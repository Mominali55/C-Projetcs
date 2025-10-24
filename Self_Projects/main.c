//Creating a Shell in C
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h> //For pid_t
#include<sys/wait.h> //For waitpid()
#include <unistd.h> // For fork(), execvp(), and chdir()

void lsh_loop(void);
char *lsh_read_line(void);
char **lsh_split_line(char *line);
int lsh_execute(char **args);
int lsh_launch(char **args);
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_num_builtins();


int main(int argc,char **argv)
{
    //Loading configuration files if any

    //Running command loop
    lsh_loop();

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
    // Note: free(line) is safe because strtok modifies line but args[i] point into line
    // This is acceptable - just be aware strtok destroys the original string
}

#define LSH_RL_BUFSIZE 1024
char *lsh_read_line(void)
{
    int bufsize =LSH_RL_BUFSIZE;
    int position =0;
    char *buffer =malloc(sizeof(char)*bufsize);
    int c;

    //If the memory allocation fails
    if(buffer == NULL) // If buffer is NULL
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

int lsh_launch(char **args)
{
    pid_t pid , wpid; //How is this valid...(Format)?-> this could be a struct datyatype ig
    int status;

    pid = fork(); //1.here as you can see it is being used "pid" 2.Remember Fork return a value 0,1,etc
    if(pid == 0){
        //child process
        if (execvp(args[0],args) == -1){ //wht is thsi Function for?
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }else if (pid < 0){
        //Error forking
        perror("lsh");  //Another Fucntion NS
    }else{
        do{
            wpid=waitpid(pid,&status,WUNTRACED);
        }while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wht i this
    }
    return 1;
}

/*
    Function Declaration for built-in shell commands:
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
    List of builtin commands,followed by their corresponding functions.
*/
char *builtin_str[]={ //Why Declaring this?And how wit is a valid format?
    "cd",
    "help",
    "exit"
}; //Here we Declare like this we will be able to add Future Builtin Commands easily

int (*builtin_func[])(char **) ={ //What is this Syntax?+ This Function named built_func that takes an array of char pointers as argument and returns an int
    //[function pointers are declared] Search this ... 
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtins(){
    return sizeof(builtin_str) /sizeof(char *);
};

/*
    Builtin function implementations.
*/
int lsh_cd(char **args){
    if (args[1] == NULL){
        fprintf(stderr,"lsh: expected argument to \"cd\"\n"); //why use \"cd\"?
        return 1; // Continue execution even if cd fails
    }else{
        if(chdir(args[1]) != 0){
            perror("lsh"); //This function is used to print a descriptive error message to stderr
        }
    }
    return 1; // we return 1 to continue executing
}

int lsh_help(char **args)
{
    int i;
    printf("Momin ali shaikh \n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++)
    {
        printf(" %s\n",builtin_str[i]); //Printing the built in commands
    }

    printf("Use the man command for information on other programs.\n");
    return 1; 
}

int lsh_exit(char **args)
{
    return 0; //**Returning 0 will cause the loop to terminate and if i return 1 it will continue executing**
}

int lsh_execute(char **args)
{
    int i;

    if (args[0] == NULL){
        //An empty command was  entered 
        return 1;
    }

        //Checking for built-in commands
        for(i=0;i<lsh_num_builtins();i++){
            if (strcmp(args[0],builtin_str[i]) == 0){
                return (*builtin_func[i])(args);
            }
        }
    return lsh_launch(args);
}




