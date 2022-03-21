#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main() {

    char* history[100];
    int historyIndex = 0;

    while (1 == 1) {

        printf("\nsish> ");

        // Get Line
        char* line = (char*) malloc(sizeof(char) * 100);
        size_t size = sizeof(line);
        getline(&line, &size, stdin);
        line[strlen(line) - 1] = '\0';

        // Parse command
        char* command[100];

        const char spaceDelimiter = ' ';
        char* save;

        int index = 0;
        char* spaceToken = strtok_r(line, &spaceDelimiter, &save);
        while (spaceToken != NULL) {

            command[index] = spaceToken;
            index += 1;
            spaceToken = strtok_r(NULL, &spaceDelimiter, &save);

        }
        
        command[index] = NULL;

        // Add command to history
        char* historyTmp = (char*) malloc(sizeof(char) * 100);
        strcpy(historyTmp, command[0]);
        history[historyIndex] = historyTmp;
        historyIndex += 1;

        // Custom commands
        if (strcmp(command[0], "exit") == 0) {

            return 0;

        } else if (strcmp(command[0], "history") == 0) {

            if (command[1] != NULL && strcmp(command[1], "-c") == 0) {

                for (int i = 0; i < historyIndex; i++) {

                        history[i] = NULL;

                }
                historyIndex = 0;

            } else {

                for (int i = 0; i < historyIndex; i++) {

                    printf("%d %s\n", i, history[i]);

                }

            }

        }

        // Run command
        int childPID = fork();

        if (childPID == 0) {

            execvp(command[0], command);

        } else {

            waitpid(childPID, 0, NULL);

        }

        free(line);

    }

    return 0;

}