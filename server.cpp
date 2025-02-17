#include <iostream>
#include <string>
#include <winsock2.h>  // Winsock API para el manejo de redes
#include <ws2tcpip.h>  // Funciones adicionales de Winsock

using namespace std;

int main() {
    // Inicializar Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Error al inicializar Winsock" << endl;
        return -1;
    }

    // Crear el socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // Crear un socket
    if (server_fd == INVALID_SOCKET) {
        cerr << "Error al crear el socket" << endl;
        WSACleanup();
        return -1;
    }

    // Configuración del servidor
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    address.sin_family = AF_INET;        // Familia de direcciones IP
    address.sin_addr.s_addr = INADDR_ANY; // Cualquier IP local
    address.sin_port = htons(9000);      // Puerto de conexión

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cerr << "Error al hacer bind" << endl;
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }

    // Escuchar conexiones
    if (listen(server_fd, 3) == SOCKET_ERROR) {
        cerr << "Error al escuchar" << endl;
        closesocket(server_fd);
        WSACleanup();
        return -1;
    }
    cout << "Servidor escuchando en el puerto 9000..." << endl;

    // Aceptar conexiones
    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket == INVALID_SOCKET) {
            cerr << "Error al aceptar conexión" << endl;
            continue;
        }

        // Obtener la IP del cliente
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(address.sin_addr), client_ip, INET_ADDRSTRLEN);
        cout << "Nuevo cliente conectado desde IP: " << client_ip << endl;

        // Aquí podrías comunicarte con el cliente...

        // Cerrar el socket del cliente
        closesocket(new_socket);
    }

    // Cerrar el socket del servidor
    closesocket(server_fd);

    // Limpiar Winsock
    WSACleanup();

    return 0;
}
