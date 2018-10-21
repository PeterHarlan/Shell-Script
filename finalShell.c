//Peter Harlan
// Include standared input and output
#include <stdio.h>
// Include standard library
#include <stdlib.h>
// Include for fork()
#include <unistd.h>
// Helps handle strings
#include <string.h>
// Include for signal handling
#include<signal.h> 
// Wait
#include<sys/wait.h>

// Maximum number of character that the user can input into command
#define LEN 80
#define HISTORY_LEN 10

// Read the user's input
int readLine(char *str, char *args[]);
// Print the user's history
void printHistory();

// Holds the indexes of the history commands
int historyCommandIndex = 0;
// Counts the numbe rof history commands up to 10
int storedHistoryCount = 0;
// Stores the number of history commands entered
unsigned long historyCommandCount = 1;

struct ProcessType
{
    /*Foreground process (value = 0), Background process (value = 1)*/
    unsigned int isBackgroundProcess : 1;
};
struct Command
{   
    int maxIndex;
    /* isBackgroundProcess defaults to false (value of 0)
       if it is a background process, value = 1*/
    unsigned int isBackgroundProcess : 1;
    /*Holds the user's input parsed as an array of strings
      command line (of 80) has max of 40 arguments */
    char *args[LEN/2 + 1];
};

// Array of commands stored as history
struct Command historyRecord[HISTORY_LEN];
struct ProcessType processType;

// Runs on trigger of signal
void handle_sigint(int sig) 
{ 
    printf("\nTo exit, type %cexit%c \n",'"', '"'); 
    printHistory();
    fflush(stdout);
} 

int main(int argc, char *argv[]){

    // Holds the user's input
    char userCommand[LEN];
    // Holds the back ground process character
    char *backgroundProcessChar = "&";
    // Holds the user's arguments
    char *args[LEN/2 + 1];
    // Last index if args
    int argsIndex = 0;

    processType.isBackgroundProcess = 0;

    // Print a welcome statement
    printf("Welcome to the Shell program\n");
    printf("Enter \"r\" to execute the last command!\n");
    printf("To execute a specific command in history, enter \"r\" x where x is the first char of the command in history.\n");
    printf("Enter <contol> + <c> to view history of commands.\n");
    printf("To exit, enter \"exit\".\n");

    // Signal Handling
    signal(SIGINT, handle_sigint);

    // Perform a continous loop
    while(1 == 1){

        fflush(stdout);
        // Prompt the user for input
        printf("osh>");
        // Flush the prompt
        fflush(stdout);

        // Scan the user's input
        argsIndex = readLine(userCommand, args);

        // Printout every element in args
        // for (int i = 0; i < argsIndex; ++i)
        // {
        //     printf("argsIndex %s\n",args[i]);
        // }

        // // if argsIndex is 0, then it will cause an error to check for & in string
        if (argsIndex < 0)
        {
            printf("No actual commands entered\n");
            fflush(stdout);
        }
        else
        {
            // Test if the user enters the word exit
            if(strcmp(args[0],"exit")==0)
            {
                printf("Exiting shell script...\n");
                exit(0);
            }
            // Check for commands with r or r x where x is any other char
            if(strcmp(args[0],"r")==0 && argsIndex < 2)
            {
                // If there exists history, re-enter it
                if(historyCommandIndex > 0){
                    // If the input only has one char of "r"
                    if(args[1] == NULL){
                        // Copy each every args in history record into args
                        for (int i = 0; i <= historyRecord[historyCommandIndex -1].maxIndex; ++i)
                        {
                            args[i] = historyRecord[historyCommandIndex - 1].args[i];
                            // printf("args[i] %s\n", args[i]);
                        }
                        // Update argsIndex
                        argsIndex = historyRecord[historyCommandIndex-1].maxIndex;
                    }
                    // If args[] has length of 2 with args[0] == r and args[1] anyother char
                    else{
                        // If there is only one char in args[2]
                        if(strlen(args[1]) == 1)
                        {   
                            // The index in the history that holds the first value beginning with the x value in "r x" input
                            int rIndex;
                            // Set to 1 if the command being searched for is in history
                            int inHistory = 0;
                            
                            // Test the first letter of every command in the history
                            for (int i = storedHistoryCount + historyCommandIndex -1; i >= historyCommandIndex; i--)
                            {
                                rIndex = i % storedHistoryCount;
                                // Gets the first character
                                char firstChar[1];
                                // Clear memory location
                                memset(firstChar, '\0', sizeof(firstChar));
                                // Copy the first character from a history record into firstChar
                                strncpy(firstChar, historyRecord[rIndex].args[0], 1);
                                firstChar[1] = '\0';

                                // If the first letter in the history command matches user's input, break from the loop
                                if(strcmp(firstChar,args[1])==0){
                                    inHistory = 1;
                                    break;
                                }
                            }
                            // Check if the searched command is in history
                            if(inHistory == 1)
                            {
                                // Copy history command to args
                                int k;
                                for (k = 0; k <= historyRecord[rIndex].maxIndex; ++k)
                                {
                                    args[k] = historyRecord[rIndex].args[k];
                                }
                                args[k] = '\0';
                                argsIndex = historyRecord[rIndex].maxIndex;
                            }
                            // Signify to the user that the command does not exit in history
                            else{
                                
                                char *error[] = {"ERROR"};
                                args[0] = error[0];
                                args[1] = NULL;
                                printf("The command you requested from history does not exist\n");
                                fflush(stdout);
                                argsIndex = 0;
                            }
                        }
                        // More than one char in args[1]
                        else{
                            // Print error command
                            printf("Input must be r x where x is any other char\n");
                        }
                    }
                }
                // Tell the user that there is no commands in the history
                else{
                    printf("No commands stored in history\n");
                }
                
            }

            // Create a fork
            pid_t pid = fork();

            // Fork fails
            if (pid < 0)
            {
                printf("Fork Failure\n");
                exit (1);
            }
            //Child process
            else if (pid == 0)
            {
                // Checks to see if the command is executed
                if (execvp(args[0], args) <= -1)
                {   
                    printf("Error executing command\n");
                    return -1;
                }
            }
            // Parent process
            else
            {
                //If it is a foreground process
                if (processType.isBackgroundProcess == 0)
                {
                    wait(NULL);
                }
            }

            // Store the max index of the word
            historyRecord[historyCommandIndex].maxIndex = argsIndex;

            // Store if input was a background process
            historyRecord[historyCommandIndex].isBackgroundProcess = processType.isBackgroundProcess;
            
            // Loop through every element in args and store it into a historyRecord
            for(int i = 0; i <= argsIndex; i++){
                // printf("args[%d] = %s\n",argsIndex, args[i]);
                historyRecord[historyCommandIndex].args[i] = malloc(sizeof(char)*(strlen(args[i])+1));
                strcpy(historyRecord[historyCommandIndex].args[i], args[i]);
            }
            storedHistoryCount++;
            if (storedHistoryCount > HISTORY_LEN)
            {
                storedHistoryCount = HISTORY_LEN;
            }
            
            // Increment the historyCommand count and increment the histoyr command index
            historyCommandCount++;
            historyCommandIndex++;
            historyCommandIndex = historyCommandIndex%HISTORY_LEN;
        }//End of else 

        // Reset the number of words
        argsIndex = 0;
    } //end of while
    // Return the main method
    return 0;
}

// Reads in the user's input
int readLine(char str[], char *args[])
{
    // Number of characters
    int numChars;

    // Index of the beginning of the command
    int startIndex = -1;

    // Index to place the next char into args[] for a command
    int argsIndex = 0; 

    // Calculate the length of the user's command
    numChars = read(STDIN_FILENO, str, LEN);

    //  If the input is less than 0, there is an invalid input
    if (numChars <= 0)
    {
        printf("Improper command...\n");
        // terminate
        exit(EXIT_FAILURE);  
    }

    // Iterate over every scanned in character
    for (int i = 0; i < numChars; ++i)
    {
        // Test each character
        switch (str[i])
        {
            // If tab, space, or new line
            // All space input by keyboard is followed by tab
            case ' ':
            case '\t':
            case '\n':
                //Test if there is a character before the tab allow args to point to it
                if(startIndex != -1)
                {
                    args[argsIndex] = &str[startIndex];    
                    argsIndex++;
                }
                // agrs[ct] can store every character from &str[start] to str[i]
                str[i] = '\0';
                // reset start index
                startIndex = -1;
                break;
            //If caracters
            default :
                // If start index is default value, set start index
                if (startIndex == -1)
                {
                    startIndex = i;
                }
        } // End of switch
    }//End of for

    if (*args[argsIndex - 1] == '&')
    {
        // Raise the background process flag
        processType.isBackgroundProcess = 1;
        // Set the last element to NULL to write over '&' and decrement argsIndex
        args[--argsIndex] = NULL;
    }
    else{
        // Set the last pointer in args to be null
        args[argsIndex] = NULL;
        // reset the backgorund process flag
        processType.isBackgroundProcess = 0;
    }
    
    // Get the max index pointing to a character instead to Null
    argsIndex--;
    // printf("Max index is %d\n", argsIndex);
    // Return the max index pointing to a string (char[])
    return argsIndex;
}

void printHistory()
{
    // The display index
    int displayIndex = 1;
    if(historyCommandCount >= 11)
    {
        displayIndex = historyCommandCount -10;
    }

    printf("\nCommand History\n");
    if(storedHistoryCount == 0)
    {
        printf("Sorry no history\n");
    }
    else
    {
        // Print each command in the order they are entered into the system
        for (int i = historyCommandIndex; i < storedHistoryCount + historyCommandIndex; i++)
        {
            printf("%d ", displayIndex);
            for(int k = 0; k <= historyRecord[(i%storedHistoryCount)].maxIndex; k++)
            {
                printf("%s ", historyRecord[(i % storedHistoryCount)].args[k]);
            }
            if(historyRecord[(i%storedHistoryCount)].isBackgroundProcess == 1)
            {
                printf("&");
            }
            printf("\n");
            displayIndex++;
        }
    }
    fflush(stdout);
    printf("osh>");
    fflush(stdout);
}