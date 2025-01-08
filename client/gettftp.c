#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

void erreur(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}


void send_rrq(int sockfd, struct addrinfo *server_info, const char *filename, const char *mode, int BUFFER_SIZE) {
    char rrq[BUFFER_SIZE];
    int rrq_len = 2 + strlen(filename) + 1 + strlen(mode) + 1;
    rrq[0] = 0x00;
    rrq[1] = 0x01;
    strcpy(rrq + 2, filename);
    strcpy(rrq + 2 + strlen(filename) + 1, mode);

    if (sendto(sockfd, rrq, rrq_len, 0, server_info->ai_addr, server_info->ai_addrlen) < 0)
        erreur("Erreur lors de l'envoi du RRQ");

    printf("Requête RRQ envoyée pour le fichier : %s\n", filename);
}

void receive_file(int sockfd, const char *filename, int BUFFER_SIZE) {
    char buffer[BUFFER_SIZE];
    FILE *output_file = fopen(filename, "wb");
    if (!output_file) erreur("Erreur lors de la création du fichier local");

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    int block_number = 0;

    while (1) {
        int received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sender_addr, &sender_len);
        if (received < 0) erreur("Erreur lors de la réception des données");

        uint16_t opcode = ntohs(*(uint16_t *)buffer);
        if (opcode == 3) {
            uint16_t received_block = ntohs(*(uint16_t *)(buffer + 2));
            if (received_block != block_number + 1) {
                fprintf(stderr, "Bloc inattendu reçu : %d\n", received_block);
                break;
            }

            fwrite(buffer + 4, 1, received - 4, output_file);

            char ack[4] = {0x00, 0x04, buffer[2], buffer[3]};
            if (sendto(sockfd, ack, sizeof(ack), 0, (struct sockaddr *)&sender_addr, sender_len) < 0)
                erreur("Erreur lors de l'envoi de l'ACK");

            printf("Bloc %d reçu et confirmé.\n", received_block);
            block_number++;

            if (received < BUFFER_SIZE) {
                printf("Dernier bloc reçu. Transfert terminé.\n");
                break;
            }
        } else if (opcode == 5) {
            fprintf(stderr, "Erreur TFTP : %s\n", buffer + 4);
            break;
        }
    }

    fclose(output_file);
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
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;


    int status = getaddrinfo(server_name, server_port, &hints, &server_info);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) erreur("Erreur lors de la création du socket");

    send_rrq(sockfd, server_info, filename, mode, BUFFER_SIZE);
    receive_file(sockfd, filename, BUFFER_SIZE);

    freeaddrinfo(server_info);
    close(sockfd);
    return 0;
}