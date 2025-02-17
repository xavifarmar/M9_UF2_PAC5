#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_IP "172.17.41.25"  // Change if the server is on another machine
#define SERVER_PORT 9000

void showMenu() {
    std::cout << "Choose an option:\n";
    std::cout << "1. Rock\n";
    std::cout << "2. Paper\n";
    std::cout << "3. Scissors\n";
    std::cout << "Enter choice: ";
}

int main() {
    int sock;
    struct sockaddr_in server;
    char choice[2];
    char server_reply[200];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Could not create socket\n";
        return 1;
    }

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connection failed\n" << SERVER_IP << std::endl;
        return 1;
    }

    std::cout << "Connected to server!\n" << SERVER_IP << std::endl;

    // Show menu
    showMenu();
    std::cin >> choice;

    // Send choice to server
    if (send(sock, choice, strlen(choice), 0) < 0) {
        std::cerr << "Send failed\n";
        return 1;
    }

    // Receive server response
    if (recv(sock, server_reply, sizeof(server_reply), 0) < 0) {
        std::cerr << "Receive failed\n";
    } else {
        std::cout << "Server says: " << server_reply << "\n";
    }

    // Close socket
    close(sock);
    return 0;
}