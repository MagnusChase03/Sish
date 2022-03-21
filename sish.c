#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{

    char *history[100];
    int historyIndex = 0;

    // Zero out history
    for (int i = 0; i < 100; i++)
    {

        history[i] = '\0';
    }

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

        // Zero out array bc sometimes has extra junk
        for (int i = 0; i < 100; i++)
        {

            for (int j = 0; j < 100; j++)
            {

                commands[i][j] = '\0';
            }
        }

        // Parse command line into seperated piped commands
        char *pipeDelim = "|\0";
        char *spaceDelim = " \0";
        char *pipeSave;
        char *spaceSave;

        int commandNum = 0;
        char *pipeToken = strtok_r(command, pipeDelim, &pipeSave);
        while (pipeToken != NULL)
        {

            int argumentNum = 0;
            char *spaceToken = strtok_r(pipeToken, spaceDelim, &spaceSave);
            while (spaceToken != NULL)
            {

                commands[commandNum][argumentNum] = spaceToken;

                argumentNum += 1;
                spaceToken = strtok_r(NULL, spaceDelim, &spaceSave);
            }

            commandNum += 1;
            pipeToken = strtok_r(NULL, pipeDelim, &pipeSave);
        }

        // Add command to history
        history[historyIndex] = command;
        historyIndex += 1;

        // Custom Commands
        if (strcmp(commands[0][0], "exit") == 0)
        {

            return 0;
        }
        else if (strcmp(commands[0][0], "cd") == 0)
        {

            if (chdir(commands[0][1]) == -1)
            {

                perror("Could not change directory");
            }
        }

        else if (strcmp(commands[0][0], "history") == 0)
        {

            if (commands[0][1] != NULL && strcmp(commands[0][1], "-c") == 0)
            {

                historyIndex = 0;
                for (int i = 0; i < 100; i++)
                {

                    history[i] = '\0';
                }
            }
            else if (commands[0][1] != NULL)
            {

                int index = atoi(commands[0][1]);
                printf("%d %s\n", index, history[index]);
            }
            else
            {

                for (int i = 0; i < historyIndex; i++)
                {

                    printf("%d %s\n", i, history[i]);
                }
            }
        }

        // If Piped command
        if (commandNum - 1 > 0)
        {

            int input = STDIN_FILENO;
            int pipe1[2];

            // Pipe the input of first command to the next strting with stdin
            for (int i = 0; i < commandNum - 1; i++)
            {

                if (pipe(pipe1) == -1)
                {

                    perror("Pipe creation failed");
                }

                pid_t childPID = fork();
                if (childPID == -1)
                {

                    perror("Fork creation failed");
                }
                else if (childPID == 0)
                {

                    if (input != STDIN_FILENO)
                    {

                        dup2(input, STDIN_FILENO);
                        close(input);
                    }

                    dup2(pipe1[1], STDOUT_FILENO);
                    close(pipe1[1]);

                    execvp(commands[i][0], commands[i]);
                    return 1;
                }
                else
                {

                    waitpid(childPID, NULL, 0);

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

                dup2(input, STDIN_FILENO);
                close(input);

                close(pipe1[0]);
                close(pipe1[1]);

                execvp(commands[commandNum - 1][0], commands[commandNum - 1]);
                return 1;
            }
            else
            {

                // Close all pipes
                waitpid(childPID, NULL, 0);

                close(input);
                close(pipe1[0]);
                close(pipe1[1]);
            }
        }
        else
        {

            // Execute Command with no pipes
            pid_t childPID = fork();
            if (childPID == -1)
            {

                perror("Fork creation failed");
            }
            else if (childPID == 0)
            {

                execvp(commands[0][0], commands[0]);
                return 1;
            }
            else
            {

                waitpid(childPID, NULL, 0);
            }
        }
    }

    return 0;
}