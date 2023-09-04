#include <iostream>
#include <cstring>
#include <winsock2.h>

using namespace std;

class Client {
public:
    WSADATA WSAData;
    SOCKET server;
    SOCKADDR_IN addr;
    char buffer[1024];

    Client(const char *serverIP) {
        cout << "Conectando al servidor..." << endl << endl;
        WSAStartup(MAKEWORD(2, 0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);
        addr.sin_addr.s_addr = inet_addr(serverIP); // Reemplaza con la dirección del servidor
        addr.sin_family = AF_INET;
        addr.sin_port = htons(5555);
        connect(server, (SOCKADDR *)&addr, sizeof(addr));
        cout << "Conectado al Servidor!" << endl;
    }

    void Enviar(const string &mensaje) {
        send(server, mensaje.c_str(), mensaje.size(), 0);
    }

    string Recibir() {
        string mensaje;
        int bytesRecibidos = recv(server, buffer, sizeof(buffer), 0);
        if (bytesRecibidos <= 0) {
            return "";
        }
        mensaje.append(buffer, bytesRecibidos);
        cout << "El servidor dice: " << mensaje << endl;
        memset(buffer, 0, sizeof(buffer));
        return mensaje;
    }

    void CerrarSocket() {
        closesocket(server);
        WSACleanup();
        cout << "Socket cerrado." << endl << endl;
    }

    ~Client() {
        closesocket(server);
        WSACleanup();
    }
};

void IniciarMenu(Client *Cliente);

int main() {
    char serverIP[] = "127.0.0.1"; // Cambia la IP del servidor si es necesario
    Client *Cliente = new Client(serverIP);
    IniciarMenu(Cliente);
    while(true){
        string opcion;
        getline(cin, opcion);
        Cliente->Enviar(opcion);
        Cliente->Recibir();
    }
    delete Cliente;
    return 0;
}

void IniciarMenu(Client *Cliente) {
    cout << "\n\nMenu de Opciones" << endl;
    cout << "1. Traductor" << endl;
    cout << "0. SALIR" << endl;
}
