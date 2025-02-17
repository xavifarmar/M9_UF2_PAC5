#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <map>
#include <mutex>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

std::mutex mtx; // To avoid race conditions

// Game rules in a map (e.g., Rock beats Scissors, etc.)
std::map<std::string, std::string> rules = {
    {"rock", "scissors"}, {"scissors", "paper"}, {"paper", "rock"},
    {"rock", "lizard"}, {"lizard", "spock"}, {"spock", "scissors"},
    {"scissors", "lizard"}, {"lizard", "paper"}, {"paper", "spock"},
    {"spock", "rock"}
};

struct Player {
    SOCKET socket;
    std::string name;
    std::string choice;
};

// Global list of players
std::vector<Player> players;

void handle_player(Player& player) {
    char buffer[1024];
    while (true) {
        // Receive choice from player
        int bytes_received = recv(player.socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0'; // Null-terminate the string
        player.choice = std::string(buffer);

        // Print player's choice
        std::cout << player.name << " chose " << player.choice << std::endl;

        // Game logic to decide winner
        std::string result = "draw";
        for (auto& other_player : players) {
            if (other_player.name != player.name) {
                if (rules[player.choice] == other_player.choice) {
                    result = "win";
                    break;
                } else if (rules[other_player.choice] == player.choice) {
                    result = "lose";
                    break;
                }
            }
        }

        std::string message = "Your result: " + result + "\n";
        send(player.socket, message.c_str(), message.length(), 0);
    }
}

std::string get_server_ip() {
    // Return the hardcoded IP address
    return "172.17.41.26";
}

void start_server(int port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket!" << std::endl;
        WSACleanup();
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Binding failed!" << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return;
    }

    listen(server_socket, 5);
    std::cout << "Server started. Waiting for clients...\n";
    std::cout << "Server IP: " << get_server_ip() << " in port " << port << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed!" << std::endl;
            continue;
        }

        // Add new player
        Player player = {client_socket, "Player_" + std::to_string(players.size() + 1), ""};
        players.push_back(player);

        std::cout << player.name << " connected!" << std::endl;

        // Handle player in a separate thread
        std::thread player_thread(handle_player, std::ref(player));
        player_thread.detach();
    }

    closesocket(server_socket);
    WSACleanup();
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // Seed random number generator
    int port = 9000; // Server port
    start_server(port);

    return 0;
}
