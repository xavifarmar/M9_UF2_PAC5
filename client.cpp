#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <regex>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return -1;
    }

    std::string server_ip;
    std::regex ip_regex(R"((\d{1,3}\.){3}\d{1,3})");
    while (true) {
        std::cout << "Enter the server IP address: ";
        std::cin >> server_ip;
        if (std::regex_match(server_ip, ip_regex)) {
            break;
        }
        std::cout << "Invalid IP address. Please try again.\n";
    }

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

    char buffer[1024];
    int bytes_received;

    std::cout << "Wait until 3 players connects to your game...\n";

    // Wait for "START" message
    while (true) {
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Connection lost or error receiving data.\n";
            closesocket(client_socket);
            WSACleanup();
            return -1;
        }
        buffer[bytes_received] = '\0';

        if (std::string(buffer) == "START") {
            std::cout << "Game is starting...\n";
            break;
        }
    }

    std::string move;
    int choice;

    while (true) {
        // Menu for the user to choose a move
        while (true) {
            std::cout << "Choose your move: \n";
            std::cout << "1. Rock\n";
            std::cout << "2. Paper\n";
            std::cout << "3. Scissors\n";
            std::cout << "4. Lizard\n";
            std::cout << "5. Spock\n";
            std::cout << "Enter the number corresponding to your choice: ";
            std::cin >> choice;

            switch (choice) {
                case 1: move = "rock"; break;
                case 2: move = "paper"; break;
                case 3: move = "scissors"; break;
                case 4: move = "lizard"; break;
                case 5: move = "spock"; break;
                default:
                    std::cout << "Invalid choice, please try again.\n";
                    continue;
            }
            break;
        }

        if (move == "quit") {
            std::cout << "Exiting the game. Goodbye!\n";
            break;
        }

        // Send the move to the server
        send(client_socket, move.c_str(), move.length(), 0);

        // Receive the result message
        bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Connection lost.\n";
            break;
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}