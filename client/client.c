#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>

#define PORT 8083
#define LOCALHOST "127.0.0.1"

int main(int argc, char *argv[]) {
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set read_fds; // for select
    char buffer[1024];
    portno = PORT;

    // create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(LOCALHOST);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    // set up server address struct
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR while connecting");
        exit(1);
    }

    printf("-------Connected to server--------\n");

    // Main loop
    while (1) {
        // Clear the fd_set and add the socket fd to it
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);  // Also monitor stdin (user input)

        // Wait for input (from either socket or stdin) with select()
        int activity = select(sockfd + 1, &read_fds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("ERROR in select");
            exit(1);
        }

        // If there is data from the server (socket is ready to read)
        if (FD_ISSET(sockfd, &read_fds)) {
            bzero(buffer, 1024);
            n = read(sockfd, buffer, 1023);
            if (n <= 0) {
                // Connection closed or error
                printf("Server closed the connection or error occurred\n");
                break;
            }
            printf("Message from server: %s\n", buffer);
        }

        // If there is user input (stdin is ready to read)
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            bzero(buffer, 1024);
            fgets(buffer, 1023, stdin); // Read input from the user

            n = write(sockfd, buffer, strlen(buffer));
            if (n < 0) {
                perror("ERROR while writing to socket");
                exit(1);
            }
        }
    }

    close(sockfd);
    return 0;
}
