#include <iostream>
#include <cstring>
#include <winsock2.h>

using namespace std;

class Server {
public:
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[1024];

    Server() {
        WSAStartup(MAKEWORD(2, 0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);

        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(5555);

        bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
        listen(server, 0);

        cout << "Escuchando para conexiones entrantes en el puerto 5555." << endl;
        int clientAddrSize = sizeof(clientAddr);
        if ((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
            cout << "Cliente conectado!" << endl;
        }
    }

    string Recibir() {
        int bytesRecibidos = recv(client, buffer, sizeof(buffer), 0);
        if (bytesRecibidos <= 0) {
            CerrarSocket();
            return "";
        }

        cout << "El cliente dice: " << buffer << endl;
        string mensaje(buffer);
        memset(buffer, 0, sizeof(buffer));
        return mensaje;
    }

    void Enviar(const string &mensaje) {
        send(client, mensaje.c_str(), mensaje.size(), 0);
        cout << "Mensaje enviado: " << mensaje << endl;
    }

    void CerrarSocket() {
        closesocket(client);
        cout << "Socket cerrado, cliente desconectado." << endl;
    }

    ~Server() {
        closesocket(server);
        WSACleanup();
    }
};

void Traductor(Server *Servidor);
string Traducir(string mensaje);

int main() {
    Server *Servidor = new Server();
    while (true) {
        string opcion = Servidor->Recibir();

        if (opcion.empty()) break; // El cliente se ha desconectado, sale del bucle

        if (opcion == "1") Traductor(Servidor);
        if (opcion == "0") Servidor->Enviar("Funcion todavia no implementada");
        if (opcion != "1" && opcion != "0") Servidor->Enviar("Inserte una opcion (Disponibles 0 y 1)");
    }
    delete Servidor;
    return 0;
}

void Traductor(Server *Servidor){
    Servidor->Enviar("Inserte una palabra en español");
    string mensaje = Servidor->Recibir();
    string traduccion = Traducir(mensaje);
    Servidor->Enviar(traduccion);
}
string Traducir(string mensaje) {
        // Traduce el mensaje a minusculas
        for (char &c : mensaje) {
            c = toupper(c);
        }
    return mensaje;
}
