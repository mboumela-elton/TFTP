#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#include "constant.c"

void printMessage(char *message)
{
    write(STDOUT_FILENO, message, strlen(message));
}

ssize_t readCommandLine(char *command)
{
    return read(STDIN_FILENO, command, sizeof(command) - 1);
}

void execSingleCommand(char *command)
{
    int status;

    char *argv[4];
    int argc = 0;

    // Diviser la commande en mots
    char *token = strtok(command, " ");
    while (token != NULL && argc < 4) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL; // Terminer le tableau d'arguments

    // Vérifier si la commande est gettftp
    if (argc == 4 && strcmp(argv[0], "gettftp") == 0) {
        // Exécuter la commande TFTP
        gettftp(argc, argv);
        perror("execvp"); // En cas d'erreur d'exécution
    } else {
        printf("Commande invalide. Utilisez : gettftp <host> <port> <file>\n");
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        
        printMessage(COMMAND_ERROR);
        _exit(1);
    }
        else if (pid > 0)
    {
        wait(&status);
    }
        else
    {
        printMessage(PROCESS_ERROR);
        printMessage(ENSEASH);
    }
}


void error(const char *msg) {
    perror(msg);
}

int gettftp(int argc, char **argv) {
    if (argc != 4) {
        printMessage("Usage: <host> <port> <file>\n");
        exit(1);
    }

    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int status = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    int sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sd == -1) {
        error("socket");
    }

    char rrq[BUFFER_SIZE] = {0};
    rrq[0] = 0;
    rrq[1] = 1; // RRQ opcode
    sprintf(rrq + 2, "%s", argv[3]);
    sprintf(rrq + 2 + strlen(argv[3]) + 1, "octet");

    ssize_t rrq_len = 2 + strlen(argv[3]) + 1 + strlen("octet") + 1;
    if (sendto(sd, rrq, rrq_len, 0, result->ai_addr, result->ai_addrlen) == -1) {
        error("sendto");
    }

    freeaddrinfo(result);

    FILE *file = fopen(argv[3], "wb");
    if (!file) {
        error("fopen");
    }

    char buffer[BUFFER_SIZE];
    struct sockaddr_storage server_addr;
    socklen_t addr_len = sizeof(server_addr);
    ssize_t n;

    while ((n = recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len)) > 0) {
        if (buffer[1] == 3) { // Data packet
            fwrite(buffer + 4, 1, n - 4, file);
            buffer[1] = 4; // ACK opcode
            if (sendto(sd, buffer, 4, 0, (struct sockaddr *)&server_addr, addr_len) == -1) {
                error("sendto");
            }
            if (n < BUFFER_SIZE) {
                break; // Last packet
            }
        } else if (buffer[1] == 5) { // Error packet
            fprintf(stderr, "Error: %s\n", buffer + 4);
            break;
        }
    }

    fclose(file);
    close(sd);

    return 0;
}