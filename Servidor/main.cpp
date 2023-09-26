#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <fstream>
#include <algorithm>  // Para transformar a minúsculas
#include <fstream>
#include <sstream>

using namespace std;

class Server {
public:
    WSADATA WSAData;
    SOCKET server;
    SOCKET client; // Cambiado a SOCKET en lugar de SOCKET client;
    SOCKADDR_IN serverAddr;
    SOCKADDR_IN clientAddr;
    char buffer[1024];

    Server() {
        WSAStartup(MAKEWORD(2, 0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);

        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(5555);

        bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
        listen(server, 0);

        cout << "Escuchando para conexiones entrantes en el puerto 5555." << endl;

        // Aceptar la conexión del cliente antes de entrar en el bucle principal
        int clientAddrSize = sizeof(clientAddr);
        if ((client = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
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

  void Enviar(const string & mensaje) {
    send(client, mensaje.c_str(), mensaje.size(), 0);
    cout << "Mensaje enviado: " << mensaje << endl;
  }

  void CerrarSocket() {
        closesocket(client);
        cout << "Socket cerrado, cliente desconectado." << endl;
        // Aceptar una nueva conexión antes de salir de esta función
        int clientAddrSize = sizeof(clientAddr);
        if ((client = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
            cout << "Cliente conectado!" << endl;
        }
    }

    ~Server() {
        closesocket(server);
        WSACleanup();
    }

  //!FUNCIONES EXTRA
  //!TRADUCTOR
  void Traductor() {
    Enviar("Inserte una palabra en ingles a traducir o escriba /salir para salir.");

    while (true) {
      string mensaje = Recibir();
      if (mensaje.empty()) break; // El cliente se ha desconectado, sale del bucle interno

      if (mensaje == "/salir") {
        Enviar("Has vuelto al menu principal.");
        break; // El usuario escribió /salir, sale del bucle interno
      }

      // Buscar la traducción en el archivo
      string traduccion = BuscarTraduccionEnArchivo(mensaje);

      // Enviar la traducción al cliente
      Enviar(traduccion);
      //Enviar("Inserte otra palabra en inglés a traducir o escriba /salir para salir.");
      //Se buguea si envio 2 mensajes seguidos asi q lo dejo comentado =)
    }
  }
  //!INSERTAR NUEVA TRADUCCION
  void InsertarNuevaTraduccion() {
    Enviar("Ingrese nueva traduccion (PalabraIngles:PalabraEspanol):");
    string nuevaTraduccion = Recibir();

    // Verificar el formato de la nueva traduccion
    size_t separadorPos = nuevaTraduccion.find(':');
    if (separadorPos == std::string::npos) {
      Enviar("No fue posible insertar la traduccion. El formato de insercion debe ser PalabraIngles:PalabraEspanol");
      return;
    }

    // Obtener la palabra en ingles y la traduccion
    string palabraIngles = nuevaTraduccion.substr(0, separadorPos);
    string traduccion = nuevaTraduccion.substr(separadorPos + 1);

    // Convertir las palabras a min�sculas
    palabraIngles = ConvertirAMinusculas(palabraIngles);
    traduccion = ConvertirAMinusculas(traduccion);

    // Verificar si la palabra ya existe en las traducciones
    ifstream archivo("traducciones.txt");
    if (archivo.is_open()) {
      string linea;
      while (getline(archivo, linea)) {
        size_t pos = linea.find(":");
        if (pos != string::npos) {
          string palabraExistente = linea.substr(0, pos);
          string traduccionExistente = linea.substr(pos + 1);
          if (ConvertirAMinusculas(palabraExistente) == palabraIngles) {
            archivo.close();
            Enviar("Ya existe una traducci�n para " + palabraIngles + ": " + traduccionExistente);
            return;
          }
        }
      }
      archivo.close();
    }

    // Agregar la nueva traducci�n al archivo en una l�nea separada con salto de l�nea
    ofstream archivoSalida("traducciones.txt", std::ios::app); // Abre el archivo en modo de apertura al final
    if (!archivoSalida.is_open()) {
      Enviar("Error al abrir el archivo de diccionario para insercion.");
      return;
    }

    archivoSalida << palabraIngles << ":" << traduccion << std::endl;
    archivoSalida.close();

    Enviar("Nueva traduccion insertada correctamente");
  }

  //!FUNCIONES AUXILIARES
  string ConvertirAMinusculas(const string & cadena) {
    string cadenaMinusculas = cadena;
    transform(cadenaMinusculas.begin(), cadenaMinusculas.end(), cadenaMinusculas.begin(), ::tolower);
    return cadenaMinusculas;
  }

  string BuscarTraduccionEnArchivo(const string & palabraIngles) {
    ifstream archivo("traducciones.txt");
    if (archivo.is_open()) {
      string palabraInglesMinusculas = ConvertirAMinusculas(palabraIngles); // Convertir la palabra a min�sculas
      string linea;
      while (getline(archivo, linea)) {
        size_t pos = linea.find(":");
        if (pos != string::npos) {
          string palabra = linea.substr(0, pos);
          string traduccion = linea.substr(pos + 1);
          if (ConvertirAMinusculas(palabra) == palabraInglesMinusculas) { // Comparar en min�sculas
            archivo.close();
            return palabraIngles + " en ingles es " + traduccion + " en espanol";
          }
        }
      }
      archivo.close();
    }
    return "No fue posible encontrar la traducci�n para:" + palabraIngles;
  }

  //!FUNCION USUARIOS
  bool AceptarCliente() {
        Enviar("Bienvenido al servidor. Por favor, ingrese su nombre de usuario:");
        string usuario = Recibir();
        Enviar("Ingrese su contraseña:");
        string contrasena = Recibir();

        // Verifica si el usuario/contraseña es valido
        if (ValidarCredenciales(usuario, contrasena)) {
            Enviar("Autenticación exitosa. ¡Bienvenido!");
            return true;
        } else {
            Enviar("Datos de usuario incorrectos. La conexión se cerrará.");
            CerrarSocket();
            return false;
        }
    }

  bool ValidarCredenciales(string usuario, string contrasena) {
    ifstream archivo("credenciales.txt");
    if (archivo.is_open()) {
        string linea;
        while (getline(archivo, linea)) {
            istringstream iss(linea);
            string usuarioArchivo, contrasenaArchivo, rol;
            int intentos;

            if (getline(iss, usuarioArchivo, '|') &&
                getline(iss, contrasenaArchivo, '|') &&
                getline(iss, rol, '|') &&
                (iss >> intentos)) {
                if (usuarioArchivo == usuario && contrasenaArchivo == contrasena) {
                    // Las credenciales son válidas
                    archivo.close();
                    return true;
                }
            }
        }
        archivo.close();
    }

    return false; // Las credenciales no son válidas o no se encontraron en el archivo
  }

};

int main() {
    Server* Servidor = new Server();
    while (true) {
        if(Servidor->AceptarCliente()){
            while (true) {
                string opcion = Servidor->Recibir();

                if (opcion.empty()) break; // El cliente se ha desconectado, sale del bucle

                else if (opcion == "1") Servidor->Traductor();
                else if (opcion == "2") Servidor->InsertarNuevaTraduccion();
                else if (opcion == "3") Servidor->Enviar("Función todavía no implementada");
                else if (opcion == "4") Servidor->Enviar("Función todavía no implementada");
                else if (opcion == "5") break;
                else Servidor->Enviar("Inserte una opción (Disponible 1 y 2)");
            }
        }

    }
    delete Servidor;
    return 0;
}
