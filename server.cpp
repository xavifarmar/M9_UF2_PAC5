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

    // Game logic loop
    while (true) {
        int bytes_received = recv(player.socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            // Desconexión del jugador
            std::lock_guard<std::mutex> lock(mtx);
            auto it = std::remove_if(players.begin(), players.end(), [&](const Player& p) {
                return p.socket == player.socket;
            });
            players.erase(it, players.end());
            closesocket(player.socket);
            std::cout << player.name << " disconnected." << std::endl;
            return;
        }

        buffer[bytes_received] = '\0';
        player.choice = std::string(buffer);
        std::cout << player.name << " chose " << player.choice << std::endl;

        // Esperar hasta que todos los jugadores hayan elegido
        bool all_players_chose = false;
        while (!all_players_chose) {
            std::lock_guard<std::mutex> lock(mtx);
            all_players_chose = true;
            for (const auto& p : players) {
                if (p.choice.empty()) {
                    all_players_chose = false;
                    break;
                }
            }
        }
    }
}

void handle_game(std::vector<Player> game_players) {
    std::cout << "All players are ready. Game starting!" << std::endl;

    // Enviar mensaje de inicio
    std::string start_message = "START";
    for (auto& p : game_players) {
        send(p.socket, start_message.c_str(), start_message.length(), 0);
        std::thread player_thread(handle_player, std::ref(p));
        player_thread.detach();
    }

    // Esperar hasta que todos los jugadores hagan su elección
    bool all_chosen = false;
    while (!all_chosen) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::lock_guard<std::mutex> lock(mtx);
        all_chosen = true;
        for (auto& p : game_players) {
            if (p.choice.empty()) {
                all_chosen = false;
                break;
            }
        }
    }

    // Determinar resultados
    std::map<std::string, int> scores;
    for (auto& p : game_players) {
        scores[p.choice]++;
    }

    // Comparar elecciones de los jugadores
    std::vector<std::string> choices;
    for (const auto& p : game_players) {
        choices.push_back(p.choice);
    }

    std::string winner = "draw";
    for (auto& p : game_players) {
        bool won = true;
        for (auto& other : game_players) {
            if (p.name != other.name) {
                if (rules[other.choice] == p.choice) {
                    won = false;
                    break;
                }
            }
        }
        if (won) {
            winner = p.name;
            break;
        }
    }

    // Enviar resultados a los jugadores
    for (auto& p : game_players) {
        std::string message = std::string("Result: ") + (winner == "draw" ? "DRAW" : (winner == p.name ? "WIN" : "LOSE")) + "\n";
        send(p.socket, message.c_str(), message.length(), 0);
    }

    std::cout << "Game over. Winner: " << winner << std::endl;
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
        std::vector<Player> game_players;
        
        while (game_players.size() < 2) { // Esperar hasta tener 3 jugadores
            SOCKET client_socket = accept(server_socket, NULL, NULL);
            if (client_socket == INVALID_SOCKET) {
                std::cerr << "Accept failed!" << std::endl;
                continue;
            }

            Player player;
            player.socket = client_socket;
            player.name = "Player_" + std::to_string(players.size() + 1);
            player.is_ready = false;

            {
                std::lock_guard<std::mutex> lock(mtx);
                players.push_back(player);
            }

            game_players.push_back(player);
            std::cout << player.name << " connected!" << std::endl;
        }

        // Iniciar un hilo para manejar el juego
        std::thread game_thread(handle_game, game_players);
        game_thread.detach(); // Para que el hilo siga ejecutándose independientemente.

        // Limpiar la lista de jugadores en la partida
        game_players.clear();
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