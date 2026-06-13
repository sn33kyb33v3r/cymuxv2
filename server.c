#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define PORT 8080

void handle_client(int client_socket) {
    char buffer[2048] = {0};
    read(client_socket, buffer, sizeof(buffer) - 1);
    
    char *body = "<html><body><h1>CYMUX_SERVER_NODE_ACTIVE</h1></body></html>";
    char response[1024];
    
    int len = snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s", 
        (int)strlen(body), body);
        
    write(client_socket, response, len);
    close(client_socket);
    exit(0);
}

int main() {
    signal(SIGCHLD, SIG_IGN);
    
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) return -1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#ifdef SO_REUSEPORT
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) return -2;
    if (listen(server_fd, 64) < 0) return -3;

    while (1) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) continue;
        
        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);
            handle_client(client_socket);
        } else {
            close(client_socket);
        }
    }
    return 0;
}
