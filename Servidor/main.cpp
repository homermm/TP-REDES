#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <fstream>

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

    //!FUNCIONES PRIMITIVAS
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

    //!FUNCIONES EXTRA
    void Traductor() {
    Enviar("Inserte una palabra en ingles a traducir");
    string palabraIngles = Recibir();

    // Buscar la traducción en el archivo y enviarla
    string traduccion = BuscarTraduccionEnArchivo(palabraIngles);

    // Enviar la traducción al cliente
    Enviar(traduccion);
    }

    string BuscarTraduccionEnArchivo(const string &palabraIngles) {
        ifstream archivo("traducciones.txt");
        if (archivo.is_open()) {
            string linea;
            while (getline(archivo, linea)) {
                size_t pos = linea.find(":");
                if (pos != string::npos) {
                    string palabra = linea.substr(0, pos);
                    string traduccion = linea.substr(pos + 1);
                    if (palabra == palabraIngles) {
                        archivo.close();
                        return palabraIngles + " en ingles es " + traduccion + " en espanol";
                    }
                }
            }
            archivo.close();
        }
        return "Palabra no encontrada en el diccionario";
    }
};


int main() {
    Server *Servidor = new Server();
    while (true) {
        string opcion = Servidor->Recibir();

        if (opcion.empty()) break; // El cliente se ha desconectado, sale del bucle

        else if (opcion == "1") Servidor->Traductor();
        else if (opcion == "2") Servidor->Enviar("Funcion todavia no implementada"); // funcion nueva trad
        else if (opcion == "3") Servidor->Enviar("Funcion todavia no implementada"); // funcion usuarios
        else if (opcion == "4") Servidor->Enviar("Funcion todavia no implementada"); // funcion registro act
        else if (opcion == "5") Servidor->Enviar("Funcion todavia no implementada"); // funcion cerrar sesion
        else if (opcion == "0") Servidor->Enviar("Funcion todavia no implementada"); // funcion salir
        else Servidor->Enviar("Inserte una opcion (Disponible 1)");
    }
    delete Servidor;
    return 0;
}



