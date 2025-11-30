#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<string.h>


// 1. Parameter is 'char you[]' (a string)
int game(char you[]) 
{
    // 2. DECLARATION: To store a LIST of words, use a 2D Array.
    // [3] = 3 words total
    // [10] = max 10 letters per word
    char computer[3][10] = {"paper", "rock", "zissor"};

    // 3. COMPARISON: Compare 'you' against each word in the list.
    // Remember: Arrays start at index 0.
    if (strcmp(you, computer[0]) == 0 || 
        strcmp(you, computer[1]) == 0 || 
        strcmp(you, computer[2]) == 0) 
    {
        return 1; // It matches one of them
    }

    return 0; // No match found
}

int main() {
    // 4. Input is a string (use double quotes "")
    char you[] = "rock"; 
    
    int result = game(you);
    printf("Draw %d", result); 
    
    return 0;
}