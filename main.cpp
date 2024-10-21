#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <ctime>
#include <string>

#define MAX_NOMS 100  // Máximo número de nombres
#define BUFFER_SIZE 100 // Tamaño del buffer para los nombres

void child_process(int proc_num, int write_fd) {
    std::string nom;

    // Solicitar un nombre al usuario
    std::cout << "Proceso " << proc_num << ": Introduce un nombre: ";
    std::getline(std::cin, nom);

    // Enviar el nombre al proceso padre a través de la tubería
    write(write_fd, nom.c_str(), nom.size() + 1); // Enviar el nombre + terminador
}

int main() {
    int N;

    // Inicializar la semilla para la función rand()
    srand(static_cast<unsigned>(time(0)));

    // Solicitar el número de procesos al usuario
    std::cout << "Introduce el número de procesos a crear: ";
    std::cin >> N;
    std::cin.ignore(); // Limpiar el buffer de entrada

    pid_t pids[N];
    int pipe_fd[2]; // Descriptores de la tubería

    // Crear la tubería
    if (pipe(pipe_fd) == -1) {
        perror("Error al crear la tubería");
        exit(1);
    }

    // Crear los procesos
    for (int i = 0; i < N; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("Error al crear el proceso");
            exit(1);
        } else if (pids[i] == 0) {
            // Este es el proceso hijo
            close(pipe_fd[0]); // Cerrar el extremo de lectura de la tubería
            child_process(i + 1, pipe_fd[1]);
            close(pipe_fd[1]); // Cerrar el extremo de escritura de la tubería
            exit(0); // Finalizar el proceso hijo
        }
    }

    // Cerrar el extremo de escritura en el padre
    close(pipe_fd[1]);

    // Proceso padre: recoger los nombres introducidos por los hijos
    std::vector<std::string> noms;
    char buffer[BUFFER_SIZE];
    
    // Leer los nombres enviados por los hijos
    for (int i = 0; i < N; i++) {
        ssize_t bytes_read = read(pipe_fd[0], buffer, BUFFER_SIZE);
        if (bytes_read > 0) {
            noms.push_back(std::string(buffer)); // Almacenar el nombre en el vector
        }
    }

    // Esperar a que terminen los procesos hijos
    for (int i = 0; i < N; i++) {
        wait(NULL);
    }

    // Cerrar el extremo de lectura en el padre
    close(pipe_fd[0]);

    // Proceso padre: mostrar los nombres recogidos
    std::cout << "\nNombres recogidos: " << std::endl;
    for (const auto& nom : noms) {
        std::cout << nom << std::endl; // Mostrar todos los nombres recogidos
    }

    // Proceso padre: seleccionar un nombre aleatorio y mostrarlo
    if (!noms.empty()) {
        int random_index = rand() % noms.size(); // Elegir un índice aleatorio
        std::cout << "\nProceso padre: El nombre seleccionado aleatoriamente es: " << noms[random_index] << std::endl;
    } else {
        std::cout << "Proceso padre: No se han añadido nombres." << std::endl;
    }

    return 0;
}
