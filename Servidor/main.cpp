#include <iostream>

#include <cstring>

#include <winsock2.h>

#include <fstream>

#include <algorithm>  // Para transformar a min�sculas

using namespace std;

class Server {
  public: WSADATA WSAData;
  SOCKET server,
  client;
  SOCKADDR_IN serverAddr,
  clientAddr;
  char buffer[1024];

  Server() {
    WSAStartup(MAKEWORD(2, 0), & WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);

    bind(server, (SOCKADDR * ) & serverAddr, sizeof(serverAddr));
    listen(server, 0);

    cout << "Escuchando para conexiones entrantes en el puerto 5555." << endl;
    int clientAddrSize = sizeof(clientAddr);
    if ((client = accept(server, (SOCKADDR * ) & clientAddr, & clientAddrSize)) != INVALID_SOCKET) {
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
      if (mensaje.empty()) {
        break; // El cliente se ha desconectado, sale del bucle interno
      }

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
    Enviar("Ingrese nueva traducci�n (PalabraIngles:PalabraEspanol):");
    string nuevaTraduccion = Recibir();

    // Verificar el formato de la nueva traducci�n
    size_t separadorPos = nuevaTraduccion.find(':');
    if (separadorPos == std::string::npos) {
      Enviar("No fue posible insertar la traducci�n. El formato de inserci�n debe ser PalabraIngles:PalabraEspanol");
      return;
    }

    // Obtener la palabra en ingl�s y la traducci�n
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
      Enviar("Error al abrir el archivo de diccionario para inserci�n.");
      return;
    }

    archivoSalida << palabraIngles << ":" << traduccion << std::endl;
    archivoSalida.close();

    Enviar("Nueva traducci�n insertada correctamente");
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
};

int main() {
  Server * Servidor = new Server();
  while (true) {
    string opcion = Servidor -> Recibir();

    if (opcion.empty()) break; // El cliente se ha desconectado, sale del bucle

    else if (opcion == "1") Servidor -> Traductor();
    else if (opcion == "2") Servidor -> InsertarNuevaTraduccion(); // funcion nueva trad
    else if (opcion == "3") Servidor -> Enviar("Funcion todavia no implementada"); // funcion usuarios
    else if (opcion == "4") Servidor -> Enviar("Funcion todavia no implementada"); // funcion registro act
    else if (opcion == "5") Servidor -> Enviar("Funcion todavia no implementada"); // funcion cerrar sesion
    else if (opcion == "0") Servidor -> Enviar("Funcion todavia no implementada"); // funcion salir
    else Servidor -> Enviar("Inserte una opcion (Disponible 1 y 2)");
  }
  delete Servidor;
  return 0;
}
