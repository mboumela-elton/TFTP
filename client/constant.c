#define BUFSIZE 1024
#define MAXDATA 512

// Définition des constantes
const char *GETTFTP = "gettftp";
const char *PUTTFTP = "puttftp";
const char *MODE = "octet"; 
const char *GETADDR_ERROR = "getaddrinfo error"; 
const char *SOCK_ERROR = "Client: 'socket' error\n"; 
const char *SENDTO_RRQ_ERROR = "Client: 'sendto RRQ' error\n";
const char *RECVFROM_ERROR = "Client: 'recvfrom' error\n";
const char *SENDTO_ACK_ERROR = "Client: 'sendto ACK' error\n";
const char *SENDTO_WRQ_ERROR = "Client: 'sendto WRQ' error\n";
const char *FILE_OPEN_ERROR = "CLIENT: Unable to open the file to send\n";
const char *SERVER_ERROR = "Client: The server returned an error message\n";
const char *BLKSIZE_OPTION = "blksize";
const char *RRQ_OPCODE = "RRQ";
const char *WRQ_OPCODE = "WRQ";
const char *DATA_OPCODE = "DATA";
const char *USAGE_MSG = "Usage: %s <command> <server> <file> <option> <blocksize>\n";
const char *CLIENT_SENT_RRQ = "CLIENT: Sent RRQ of %d bytes to the server\n";
const char *CLIENT_RECEIVED_PACKET = "CLIENT: Received packet number %d of %d bytes\n";
const char *CLIENT_ACK_SENT = "CLIENT: Acknowledgment sent for packet number %d\n";
const char *CLIENT_SENT_WRQ = "CLIENT: Sent WRQ of %d bytes to the server\n";
const char *CLIENT_ACK_WRQ_RECEIVED = "CLIENT: Acknowledgment for WRQ received from server: %d%d|%d%d\n";
const char *CLIENT_FILE_TRANSFER_COMPLETE = "CLIENT: File transfer complete\n";
const char *CLIENT_FILE_TRANSFER_TO_SERVER_COMPLETE = "CLIENT: File transfer to the server completed\n";