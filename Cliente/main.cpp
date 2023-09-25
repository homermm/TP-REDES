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

    Client(const char* serverIP, int serverPort) {
        cout << "Conectando al servidor..." << endl << endl;
        WSAStartup(MAKEWORD(2, 0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);
        if (server == INVALID_SOCKET) {
            cerr << "Error al crear el socket." << endl;
            WSACleanup();
            exit(1);
        }
        addr.sin_addr.s_addr = inet_addr(serverIP); //127.0.0.1 default
        addr.sin_family = AF_INET;
        addr.sin_port = htons(serverPort); //5555 default
        if (connect(server, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            cerr << "Error al conectar al servidor. Verifique la direccion IP y el puerto." << endl;
            closesocket(server);
            WSACleanup();
            exit(1);
        }
        cout << "Conectado al Servidor!" << endl;
    }

    void Enviar(const string& mensaje) {
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

    void MostrarMenu() {
        system("cls");
        cout << "Menu de Opciones" << endl;
        cout << "1. Traducir" << endl;
        cout << "2. Nueva traducción" << endl;
        cout << "3. Usuarios" << endl;
        cout << "4. Ver registro de actividades" << endl;
        cout << "5. Cerrar sesión" << endl;
        cout << "0. SALIR" << endl;
    }

    ~Client() {
        closesocket(server);
        WSACleanup();
    }
};

int main() {
    string serverIP;
    int serverPort;

    cout << "Ingrese la direccion IP del servidor: ";
    cin >> serverIP;
    cout << "Ingrese el puerto del servidor: ";
    cin >> serverPort;

    cin.ignore(); // Limpia el buffer de entrada

    Client* Cliente = new Client(serverIP.c_str(), serverPort);

    Cliente->MostrarMenu();
    while (true) {
        string opcion;
        getline(cin, opcion);

        Cliente->Enviar(opcion);
        if (opcion == "/salir") {
            Cliente->MostrarMenu();
        }
        Cliente->Recibir();
    }

    delete Cliente;
    return 0;
}
