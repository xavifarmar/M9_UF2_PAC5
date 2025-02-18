#include <iostream>
#include <vector>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <map>
#include <mutex>
#include <algorithm>
#include <regex>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library

std::mutex mtx; // Mutex for thread safety

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
    bool is_ready = false; // Track if the player is ready
};

// Global list of players
std::vector<Player> players;

void handle_player(Player& player) {
    Player recievedPlayer = player;
    char buffer[1024];

    // First loop: wait for the player to send "ready" or "not ready"
    while (true) {
        int bytes_received = recv(player.socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = std::remove_if(players.begin(), players.end(), [&](const Player& p) {
                return p.socket == player.socket;
            });
            players.erase(it, players.end());
            closesocket(player.socket);
            std::cout << recievedPlayer.name << " disconnected." << std::endl;
            return;  // Exit the function, player disconnected
        }
        buffer[bytes_received] = '\0'; // Null-terminate the string
        std::string response(buffer);

        // If the player is "ready", exit the first loop and start the game
        if (response == "ready") {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& p : players) {
                if (p.name == recievedPlayer.name) {
                    p.is_ready = true;
                    std::cout << p.name << " is ready!" << std::endl;
                }
            }
            break;  // Exit the ready check loop when the player is ready
        } else if (response == "not ready") {
            std::cout << recievedPlayer.name << " is not ready." << std::endl;
        }
    }

    // Game logic loop
    while (true) {
        int bytes_received = recv(player.socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = std::remove_if(players.begin(), players.end(), [&](const Player& p) {
                return p.socket == player.socket;
            });
            players.erase(it, players.end());
            closesocket(player.socket);
            std::cout << recievedPlayer.name << " disconnected." << std::endl;
            return;  // Exit the function, player disconnected
        }
        buffer[bytes_received] = '\0'; // Null-terminate the string
        player.choice = std::string(buffer);

        std::cout << player.name << " chose " << player.choice << std::endl;

        // Game logic to decide winner
        std::string result = "draw";
        {
            std::lock_guard<std::mutex> lock(mtx);
            for (auto& other_player : players) {
                if (other_player.name != player.name && !other_player.choice.empty()) {
                    if (rules[player.choice] == other_player.choice) {
                        result = "win";
                        break;
                    } else if (rules[other_player.choice] == player.choice) {
                        result = "lose";
                        break;
                    }
                }
            }
        }

        std::string message = "Your result: " + result + "\n";
        send(player.socket, message.c_str(), message.length(), 0);

        // Wait for all players to finish the game before starting a new one
        std::this_thread::sleep_for(std::chrono::seconds(2)); // 2 seconds pause before the next round
    }
}

std::string get_server_ip() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        std::cerr << "Error getting hostname" << std::endl;
        return "";
    }

    struct addrinfo hints, *info;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;  // Only IPv4 addresses
    hints.ai_socktype = SOCK_STREAM;

    int res = getaddrinfo(hostname, NULL, &hints, &info);
    if (res != 0) {
        std::cerr << "getaddrinfo failed: " << gai_strerror(res) << std::endl;
        return "";
    }

    std::string ip_address;
    for (struct addrinfo* ptr = info; ptr != NULL; ptr = ptr->ai_next) {
        sockaddr_in* sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
        ip_address = inet_ntoa(sockaddr_ipv4->sin_addr);
        break;  // We are only interested in the first valid IPv4 address
    }

    freeaddrinfo(info);
    return ip_address;
}

void check_all_ready() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Prevents high CPU usage

        if (!players.empty() && std::all_of(players.begin(), players.end(), [](const Player& p) { return p.is_ready; })) {
            std::cout << "All players are ready. Game starting!" << std::endl;
            std::string start_message = "START";

            for (auto& p : players) {
                send(p.socket, start_message.c_str(), start_message.length(), 0);
                p.is_ready = false; // Reset for next round
            }
        }
    }
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
        Player player = {client_socket, "Player_" + std::to_string(players.size() + 1), "", false};
        {
            std::lock_guard<std::mutex> lock(mtx);
            players.push_back(player);
        }
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

    std::thread ready_checker(check_all_ready); // Start the thread to monitor readiness
    ready_checker.detach();

    start_server(port);
    return 0;
}