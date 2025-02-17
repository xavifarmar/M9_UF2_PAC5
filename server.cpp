#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>  // Para obtener la IP de las interfaces de red
#include <cstring>    // Para usar strcmp
#include <netdb.h>    // Para getaddrinfo()

using namespace std;

// Función para obtener la IP del servidor
string getServerIP() {
    struct ifaddrs *interfaces, *ifa;
    void *tmpAddrPtr = NULL;
    string ipAddress;

    if (getifaddrs(&interfaces) == 0) {
        for (ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next) {
            // Buscar una interfaz que tenga una dirección IPv4
            if (ifa->ifa_addr->sa_family == AF_INET) {
                // Convertir la dirección IPv4 a una cadena
                tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);

                // Si es la interfaz que está activa y no es la de "lo0" (localhost), la devolvemos
                if (ifa->ifa_name != NULL && strcmp(ifa->ifa_name, "lo0") != 0) {
                    ipAddress = ip;
                    break;
                }
            }
        }
        freeifaddrs(interfaces);
    }

    return ipAddress.empty() ? "No se pudo obtener la IP del servidor" : ipAddress;
}

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

    // Mostrar la IP del servidor usando getaddrinfo() (en caso de que no se obtenga con getifaddrs())
    string serverIP = getServerIP();
    if (serverIP == "No se pudo obtener la IP del servidor") {
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;  // IPv4
        hints.ai_socktype = SOCK_STREAM;

        // Obtener la IP del servidor usando getaddrinfo()
        int status = getaddrinfo(NULL, "9000", &hints, &res);
        if (status == 0) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) res->ai_addr;
            inet_ntop(AF_INET, &(sockaddr_ipv4->sin_addr), ip, INET_ADDRSTRLEN);
            serverIP = string(ip);
            freeaddrinfo(res);
        }
    }

    cout << "Servidor iniciado en IP: " << serverIP << " en el puerto 9000..." << endl;

    // Aceptar conexiones
    struct sockaddr_in client_address;
    socklen_t client_addrlen = sizeof(client_address);

    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr*)&client_address, &client_addrlen);
        if (new_socket < 0) {
            cerr << "Error al aceptar conexión" << endl;
            continue; // Continuar escuchando nuevas conexiones
        }

        // Obtener la IP del cliente
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_ip, INET_ADDRSTRLEN);
        cout << "Nuevo cliente conectado desde IP: " << client_ip << endl;

        // Aquí podrías hacer algo más, como comunicarte con el cliente...

        // Cerrar el socket de cliente (si no es necesario mantener la conexión abierta)
        close(new_socket);
    }

    close(server_fd);

    return 0;
}
