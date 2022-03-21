#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

// Clear out history
void clearHist(char *history[100], int *historyIndex)
{

    *historyIndex = 0;
    for (int i = 0; i < 100; i++)
    {

        history[i] = '\0';
    }
}

// Clear commands
void clearCommands(char *commands[100][100])
{

    for (int i = 0; i < 100; i++)
    {

        for (int j = 0; j < 100; j++)
        {

            commands[i][j] = '\0';
        }
    }
}

// Execute a simple command
void executeCommand(char *command[100])
{

    // Fork
    pid_t childPID = fork();
    if (childPID == -1)
    {

        perror("Fork creation failed");
    }
    else if (childPID == 0)
    {

        // Execute command
        execvp(command[0], command);
        perror("No command found");
    }
    else
    {

        waitpid(childPID, NULL, 0);
    }
}

// Parse into command
void parseComand(char *commands[100][100], char *command, int *commandNum)
{

    char *pipeDelim = "|\0";
    char *spaceDelim = " \0";
    char *pipeSave;
    char *spaceSave;

    // Split on |
    char *pipeToken = strtok_r(command, pipeDelim, &pipeSave);
    while (pipeToken != NULL)
    {

        // Split on spaces
        int argumentNum = 0;
        char *spaceToken = strtok_r(pipeToken, spaceDelim, &spaceSave);
        while (spaceToken != NULL)
        {

            // Add the command argument to the list
            commands[*commandNum][argumentNum] = spaceToken;

            argumentNum += 1;
            spaceToken = strtok_r(NULL, spaceDelim, &spaceSave);
        }

        *commandNum += 1;
        pipeToken = strtok_r(NULL, pipeDelim, &pipeSave);
    }
}

// Handle custom commands
int executeCustomCommand(char *command[100], char *history[100], int *historyIndex)
{

    // Exit Command
    if (strcmp(command[0], "exit") == 0)
    {

        return 2;
    }

    // Cd Command
    else if (strcmp(command[0], "cd") == 0)
    {

        if (chdir(command[1]) == -1)
        {

            perror("Could not change directory");
            return 0;
        }
        return 0;
    }

    // History command
    else if (strcmp(command[0], "history") == 0)
    {

        // Clear history
        if (command[1] != NULL && strcmp(command[1], "-c") == 0)
        {

            clearHist(history, historyIndex);
            return 0;
        }

        // Grab history index
        else if (command[1] != NULL)
        {

            int index = atoi(command[1]);

            if (index < *historyIndex)
            {

                printf("%d %s\n", index, history[index]);
                return 0;
            }
            else
            {

                printf("Index out of range\n");
                return 0;
            }
        }

        // Print history
        else
        {

            for (int i = 0; i < *historyIndex; i++)
            {

                printf("%d %s\n", i, history[i]);
            }

            return 0;
        }
        return 1;
    }
}

int main()
{

    char *history[100];
    int historyIndex = 0;
    clearHist(history, &historyIndex);

    // Shell Loop
    while (1 == 1)
    {

        printf("\nsish> ");

        // Get inputed command
        char *command = malloc(sizeof(char) * 100);
        size_t commandSize = sizeof(command);
        getline(&command, &commandSize, stdin);
        command[strlen(command) - 1] = '\0';

        // Parse command line into seperated piped commands
        char *commands[100][100];
        clearCommands(commands);

        int commandNum = 0;
        parseComand(commands, command, &commandNum);

        // Add command to history
        history[historyIndex] = command;
        historyIndex += 1;

        // Custom Commands
        int retVal = executeCustomCommand(commands[0], history, &historyIndex);
        if (retVal == 2)
        {

            return 0;
        }
        else if (retVal == 0)
        {

            continue;
        }

        // If Piped command
        if (commandNum - 1 > 0)
        {

            int input = STDIN_FILENO;
            int pipe1[2];

            // Pipe the input of first command to the next strting with stdin
            for (int i = 0; i < commandNum - 1; i++)
            {

                // Create pipe
                if (pipe(pipe1) == -1)
                {

                    perror("Pipe creation failed");
                }

                // Create fork
                pid_t childPID = fork();
                if (childPID == -1)
                {

                    perror("Fork creation failed");
                }
                else if (childPID == 0)
                {

                    // Set input to last pipe and close
                    if (input != STDIN_FILENO)
                    {

                        dup2(input, STDIN_FILENO);
                        close(input);
                    }

                    // Set output to pipe and close
                    dup2(pipe1[1], STDOUT_FILENO);
                    close(pipe1[1]);

                    // Execute Command
                    execvp(commands[i][0], commands[i]);
                    perror("No command found");
                }
                else
                {

                    waitpid(childPID, NULL, 0);

                    // Close input
                    if (input != STDIN_FILENO)
                    {

                        close(input);
                    }

                    // Set input to be the output
                    input = pipe1[0];
                    close(pipe1[1]);
                }
            }

            // Execute last command to stdout
            pid_t childPID = fork();
            if (childPID == -1)
            {

                perror("Fork creation failed");
            }
            else if (childPID == 0)
            {

                // Grab input and close pipes
                dup2(input, STDIN_FILENO);
                close(input);

                close(pipe1[0]);
                close(pipe1[1]);

                // Execute command
                execvp(commands[commandNum - 1][0], commands[commandNum - 1]);
                perror("No command found");
            }
            else
            {

                waitpid(childPID, NULL, 0);

                // Close all pipes
                close(input);
                close(pipe1[0]);
                close(pipe1[1]);
            }
        }
        else
        {

            // Execute Command with no pipes
            executeCommand(commands[0]);
        }
    }

    return 0;
}