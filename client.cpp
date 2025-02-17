#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return -1;
    }

    std::string server_ip = "172.17.41.25"; // Hardcoded server IP
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket!" << std::endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9000); // Server port
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server!" << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return -1;
    }

    std::string move;
    std::cout << "Enter your choice (rock, paper, scissors, lizard, spock): ";
    std::cin >> move;

    send(client_socket, move.c_str(), move.length(), 0);

    char buffer[1024];
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    buffer[bytes_received] = '\0'; // Null-terminate the string
    std::cout << buffer << std::endl;

    closesocket(client_socket); // Close the socket
    WSACleanup(); // Clean up Winsock
    return 0;
}