#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <map>
#include <mutex>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

std::mutex mtx;
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
    bool ready = false;
};

std::vector<Player> players;

void check_and_start_game() {
    std::lock_guard<std::mutex> lock(mtx);
    if (players.size() == 2 && players[0].ready && players[1].ready) {
        std::cout << "Both players are ready! Game starting..." << std::endl;
        for (auto& player : players) {
            std::string start_message = "START";
            send(player.socket, start_message.c_str(), start_message.length(), 0);
        }
    }
}

void handle_player(Player& player) {
    char buffer[1024];
    while (true) {
        int bytes_received = recv(player.socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) break;
        buffer[bytes_received] = '\0';
        std::string message(buffer);
        
        if (message == "ready") {
            std::lock_guard<std::mutex> lock(mtx);
            player.ready = true;
            std::cout << player.name << " is ready!" << std::endl;
            check_and_start_game();
        } else if (message == "not ready") {
            std::lock_guard<std::mutex> lock(mtx);
            player.ready = false;
            std::cout << player.name << " is not ready!" << std::endl;
        } else {
            player.choice = message;
        }
    }
}

void start_server(int port) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 2);
    std::cout << "Server started. Waiting for clients...\n";

    while (players.size() < 2) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        Player player = {client_socket, "Player_" + std::to_string(players.size() + 1), ""};
        players.push_back(player);
        std::cout << player.name << " connected!" << std::endl;
        std::thread player_thread(handle_player, std::ref(players.back()));
        player_thread.detach();
    }
    closesocket(server_socket);
    WSACleanup();
}

int main() {
    int port = 9000;
    start_server(port);
    return 0;
}