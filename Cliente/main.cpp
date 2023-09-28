#include <iostream>
#include <cstring>
#include <winsock2.h>

using namespace std;

class Client {
  public: WSADATA WSAData;
  SOCKET server;
  SOCKADDR_IN addr;
  char buffer[1024];

  Client(const char * serverIP, int serverPort,
    const char * usuario,
      const char * contrasena) {
    cout << "Conectando al servidor..." << endl << endl;
    WSAStartup(MAKEWORD(2, 2), & WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) {
      cerr << "Error al crear el socket." << endl;
      WSACleanup();
      exit(1);
    }
    addr.sin_addr.s_addr = inet_addr(serverIP);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(serverPort);
    if (connect(server, (SOCKADDR * ) & addr, sizeof(addr)) == SOCKET_ERROR) {
      cerr << "Error al conectar al servidor. Verifique la dirección IP y el puerto." << endl;
      closesocket(server);
      WSACleanup();
      exit(1);
    }
    cout << "Conectado al Servidor!" << endl;

    // Recibir respuesta del servidor para el usuario
    Recibir();

    // Enviar el usuario al servidor
    Enviar(usuario);

    // Recibir respuesta del servidor para la contraseña
    Recibir();

    // Enviar la contraseña al servidor
    Enviar(contrasena);
  }

  void Enviar(const string & mensaje) {
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

int main() {
  string serverIP;
  int serverPort;

  cout << "Ingrese la direccion IP del servidor: ";
  cin >> serverIP;
  cout << "Ingrese el puerto del servidor: ";
  cin >> serverPort;

  string usuario, contrasena;
  cout << "Ingrese su nombre de usuario: ";
  cin >> usuario;
  cout << "Ingrese su contrasena: ";
  cin >> contrasena;

  cin.ignore(); // Limpia el buffer de entrada

  Client * Cliente = new Client(serverIP.c_str(), serverPort, usuario.c_str(), contrasena.c_str());

  Cliente -> Recibir(); // mensaje de autenticacion correcta
  //system("cls"); // limpio el inicio de sesion
  Cliente -> Recibir(); // recibo menu

  while (true) {
    string opcion;
    getline(cin, opcion);
    Cliente -> Enviar(opcion);

    if (opcion == "/salir") system("cls"); // borro la consola para luego recibir el menu desde el servidor

    Cliente -> Recibir();
  }

  delete Cliente;
  return 0;
}
