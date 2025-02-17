#include <iostream>
#include <string>
#include <sys/socket.h>  // Funciones de socket en Linux
#include <arpa/inet.h>   // Funciones de dirección y conversión IP
#include <unistd.h>      // Funciones para manejo de sockets

using namespace std;

int main() {
    // Crear el socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        cerr << "Error al crear el socket" << endl;
        return -1;
    }

    // Configuración del servidor
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    address.sin_family = AF_INET; // Familia de direcciones IP
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9000); // Puerto de conexión

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Error al hacer bind" << endl;
        return -1;
    }

    // Escuchar conexiones
    if (listen(server_fd, 3) < 0) {
        cerr << "Error al escuchar" << endl;
        return -1;
    }

    cout << "Servidor escuchando en el puerto 9000..." << endl;

    // Aceptar conexiones
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);
    int new_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
    if (new_socket < 0) {
        cerr << "Error al aceptar conexión" << endl;
        return -1;
    }

    // Obtener la IP del cliente
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);

    // Mostrar la IP del cliente
    cout << "Nuevo cliente conectado desde IP: " << client_ip << endl;

    // Aquí podrías hacer algo más, como comunicarte con el cliente...

    // Cerrar el socket
    close(new_socket);
    close(server_fd);

    return 0;
}
