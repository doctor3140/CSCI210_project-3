#include <stdio.h>
#include <stdlib.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define N 12

extern char **environ;

char *allowed[N] = {"cp","touch","mkdir","ls","pwd","cat","grep","chmod","diff","cd","exit","help"};

int isAllowed(const char *cmd) {
    for (int i = 0; i < N; i++) {
        if (strcmp(cmd, allowed[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    char line[256];
    char *argv[21];  // Maximum of 20 arguments + 1 for NULL

    while (1) {
        fprintf(stderr, "rsh> ");

        if (fgets(line, sizeof(line), stdin) == NULL) continue;
        if (strcmp(line, "\n") == 0) continue;
        line[strlen(line) - 1] = '\0';  // Remove newline

        // Tokenize input
        int argc = 0;
        char *token = strtok(line, " ");
        while (token != NULL && argc < 20) {
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        if (argc == 0) continue;

        // Check if the command is allowed
        if (!isAllowed(argv[0])) {
            printf("NOT ALLOWED!\n");
            continue;
        }

        // Handle built-in commands
        if (strcmp(argv[0], "cd") == 0) {
            if (argc > 2) {
                printf("-rsh: cd: too many arguments\n");
            } else if (argc == 2 && chdir(argv[1]) != 0) {
                perror("cd failed");
            }
        } else if (strcmp(argv[0], "exit") == 0) {
            return 0;
        } else if (strcmp(argv[0], "help") == 0) {
            printf("The allowed commands are:\n");
            for (int i = 0; i < N; i++) {
                printf("%d: %s\n", i+1,allowed[i]);
            }
        }
        // Handle external commands (spawned processes)
        else {
            pid_t pid;
            posix_spawnattr_t attr;
            posix_spawnattr_init(&attr);

            if (posix_spawnp(&pid, argv[0], NULL, &attr, argv, environ) != 0) {
                perror("spawn failed");
            } else {
                int status;
                waitpid(pid, &status, 0);
            }

            posix_spawnattr_destroy(&attr);
        }
    }
    return 0;
}
