#include <iostream>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;


int main(){

int server_fd = socket(AF_INET, SOCK_STREAM, 0); //Crear un socket

if (server_fd == -1) {
 	cerr << "Error al crear el socket" << endl;
 	return -1;
 }

//CONFIGURACION SERVER

struct sockaddr_in address;
int addrlen = sizeof(address);

address.sin_family = AF_INET; //Familia de direcciones IP
address.sin_addr.s_addr = INADDR_ANY;
address.sin_port = htons(9000); // Puerto de conexion

if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
	cerr << "Error al hacer bind" << endl;
	return -1;
}

//Escuchar conexiones

if (listen(server_fd, 3) < 0) {
    cerr << "Error al escuchar" << endl;
    return -1;
}
cout << "Servidor escuchando en el puerto 9000..." << endl;


//Aceptar conexiones
while (true){

	int new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
	if (new_socket < 0) {
    	cerr << "Error al aceptar conexión" << endl;
	continue;
	}
	 cout << "Nuevo cliente conectado" << endl;
 close(new_socket);
}

close(server_fd);
return 0;



}
