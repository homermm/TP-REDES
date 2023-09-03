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

    void Enviar() {
        cout << "Escribe el mensaje a enviar: ";
        cin.getline(buffer, sizeof(buffer));
        send(server, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
        cout << "Mensaje enviado!" << endl;
    }

    string Recibir() {
        recv(server, buffer, sizeof(buffer), 0);
        cout << "El servidor dice: " << buffer << endl;
        string mensaje(buffer);
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

int main() {
    char serverIP[16];
    cout << "Ingrese la IP del servidor: ";
    cin.getline(serverIP, sizeof(serverIP));
    Client *Cliente = new Client(serverIP);
    while (true) {
        Cliente->Enviar();
        string mensaje = Cliente->Recibir();
    }
    delete Cliente;
    return 0;
}
