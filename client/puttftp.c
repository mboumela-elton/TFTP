#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define DATA_SIZE 512

void error(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

// Send WRQ (Write Request) packet to the server and receive the initial ACK (Acknowledgement)
struct sockaddr_in send_wrq(int sockfd, struct addrinfo *server_info, const char *filename, const char *mode, int BUFFER_SIZE) {
    char wrq[BUFFER_SIZE];
    int wrq_len = 2 + strlen(filename) + 1 + strlen(mode) + 1;

    wrq[0] = 0x00; 
    wrq[1] = 0x02;
    strcpy(wrq + 2, filename);
    strcpy(wrq + 2 + strlen(filename) + 1, mode);

    if (sendto(sockfd, wrq, wrq_len, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
        error("Error sending WRQ");

    printf("WRQ request sent for file: %s\n", filename);

    char buffer[BUFFER_SIZE];
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    int received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sender_addr, &sender_len);
    if (received < 0) error("Error receiving initial ACK");

    uint16_t opcode = ntohs(*(uint16_t *)buffer);
    uint16_t block_number = ntohs(*(uint16_t *)(buffer + 2));

    if (opcode != 4 || block_number != 0) {
        fprintf(stderr, "Error: Unexpected ACK (block %d, opcode %d).\n", block_number, opcode);
        exit(EXIT_FAILURE);
    }

    printf("Initial ACK received (block 0).\n");
    return sender_addr;
}

// Send the file to the server
void send_file(int sockfd, struct sockaddr_in *server_addr, const char *filename, int BUFFER_SIZE) {
    FILE *input_file = fopen(filename, "rb");
    if (!input_file) error("Error opening local file");

    char buffer[BUFFER_SIZE];
    char ack[4];
    int block_number = 0;
    socklen_t server_len = sizeof(struct sockaddr_in);

    while (1) {
        int data_len = fread(buffer + 4, 1, DATA_SIZE, input_file);
        if (data_len < 0) error("Error reading file");

        buffer[0] = 0x00;
        buffer[1] = 0x03; 
        buffer[2] = (block_number + 1) >> 8;  
        buffer[3] = (block_number + 1) & 0xFF; 

        int sent = sendto(sockfd, buffer, data_len + 4, 0, (struct sockaddr *)server_addr, server_len);
        if (sent < 0) error("Error sending DATA packet");

        printf("Block %d sent (%d bytes).\n", block_number + 1, data_len);

        int received = recvfrom(sockfd, ack, sizeof(ack), 0, NULL, NULL);
        if (received < 0) error("Error receiving ACK");

        uint16_t opcode = ntohs(*(uint16_t *)ack);
        uint16_t ack_block = ntohs(*(uint16_t *)(ack + 2));

        if (opcode != 4 || ack_block != block_number + 1) {
            fprintf(stderr, "Error: Unexpected ACK (block %d, opcode %d).\n", ack_block, opcode);
            break;
        }

        printf("ACK received for block %d.\n", ack_block);
        block_number++;

        if (data_len < DATA_SIZE) {
            printf("Last block sent. Transfer completed.\n");
            break;
        }
    }

    fclose(input_file);
}

int main(int argc, char *argv[]) {
    const char *server_name = argv[1];
    const char *server_port = argv[2];;
    const char *filename = argv[3];;
    const char *mode = "octet";
    int BUFFER_SIZE;

    if(argc < 5){
        BUFFER_SIZE = 512;
    }else{
        BUFFER_SIZE = atoi(argv[4]);
    }

    // Get the server address information
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int status = getaddrinfo(server_name, server_port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) error("Error creating socket");

    struct sockaddr_in new_sock = send_wrq(sockfd, res, filename, mode, BUFFER_SIZE);
    
    send_file(sockfd, &new_sock, filename, BUFFER_SIZE);

    freeaddrinfo(res);
    close(sockfd);
    return 0;
}
