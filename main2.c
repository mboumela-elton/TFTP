#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 516

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <host> <port> <file>\n", argv[0]);
        exit(EXIT_FAILURE);
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