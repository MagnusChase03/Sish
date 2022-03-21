#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main() {

    // char* history[100];
    // int historyIndex = 0;

    // while (1 == 1) {

    //     printf("\nsish> ");

    //     // Get Line
    //     char* line = (char*) malloc(sizeof(char) * 100);
    //     size_t size = sizeof(line);
    //     getline(&line, &size, stdin);
    //     line[strlen(line) - 1] = '\0';

    //     // Parse command
    //     char* command[100];

    //     const char spaceDelimiter = ' ';
    //     char* save;

    //     int index = 0;
    //     char* spaceToken = strtok_r(line, &spaceDelimiter, &save);
    //     while (spaceToken != NULL) {

    //         command[index] = spaceToken;
    //         index += 1;
    //         spaceToken = strtok_r(NULL, &spaceDelimiter, &save);

    //     }
        
    //     command[index] = NULL;

    //     // Add command to history
    //     char* historyTmp = (char*) malloc(sizeof(char) * 100);
    //     strcpy(historyTmp, command[0]);
    //     history[historyIndex] = historyTmp;
    //     historyIndex += 1;

    //     // Custom commands
    //     if (strcmp(command[0], "exit") == 0) {

    //         return 0;

    //     } else if (strcmp(command[0], "history") == 0) {

    //         if (command[1] != NULL && strcmp(command[1], "-c") == 0) {

    //             for (int i = 0; i < historyIndex; i++) {

    //                     history[i] = NULL;

    //             }
    //             historyIndex = 0;

    //         } else {

    //             for (int i = 0; i < historyIndex; i++) {

    //                 printf("%d %s\n", i, history[i]);

    //             }

    //         }

    //     }

    //     // Run command
    //     int childPID = fork();

    //     if (childPID == 0) {

    //         execvp(command[0], command);

    //     } else {

    //         waitpid(childPID, 0, NULL);

    //     }

    //     free(line);

    // }

    // Shell Loop
    while (1 == 1) {

        printf("\nsish> ");

        // Get command line
        char* command = malloc(sizeof(char) * 100);
        size_t commandSize = sizeof(command);
        getline(&command, &commandSize, stdin);
        command[strlen(command) - 1] = '\0';

        // Parse command line into seperated piped commands
        char* commands[100][100];

        //Zero out array bc sometimes has extra junk
        for (int i = 0; i < 100; i++) {

            for (int j = 0; j < 100; j++) {

                commands[i][j] = '\0';

            }

        }

        // Parse command line into seperated piped commands
        char* pipeDelim = "|\0";
        char* spaceDelim = " \0";
        char* pipeSave;
        char* spaceSave;

        int commandNum = 0;
        char* pipeToken = strtok_r(command, pipeDelim, &pipeSave);
        while (pipeToken != NULL) {

            int argumentNum = 0;
            char* spaceToken = strtok_r(pipeToken, spaceDelim, &spaceSave);
            while (spaceToken != NULL) {

                commands[commandNum][argumentNum] = spaceToken;

                argumentNum += 1;
                spaceToken = strtok_r(NULL, spaceDelim, &spaceSave);

            }

            commandNum += 1;
            pipeToken = strtok_r(NULL, pipeDelim, &pipeSave);

        }

        // If Piped command
        if (commandNum - 1 > 0) {

            int input, pipe1[2];
            input = STDIN_FILENO;

            for (int j = 0; j < commandNum - 1; j++) {

                pipe(pipe1);
                int childPID = fork();

                if (childPID == 0) {

                    if (input != STDIN_FILENO) {

                        dup2(input, STDIN_FILENO);
                        close(input);

                    }

                    dup2(pipe1[1], STDOUT_FILENO);
                    close(pipe1[1]);

                    execvp(commands[j][0], commands[j]);

                }

                close(input);
                close(pipe1[1]);
                input = pipe1[0];

            }

            if (input != STDIN_FILENO) {

                dup2(input, STDIN_FILENO);
                close(input);

            }

            execvp(commands[commandNum - 1][0], commands[commandNum - 1]);

        } else {

            // Execute Command with no pipes
            pid_t childPID = fork();
            if (childPID == 0) {

                execvp(commands[0][0], commands[0]);

            } else {

                waitpid(childPID, NULL, 0);

            }

        }

        free(command);


    }

    return 0;

}