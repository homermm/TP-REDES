#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <fstream>  // Agregado para trabajar con archivos
#include <algorithm> // Agregado para transformar a minúsculas
#include <sstream>
#include <vector>
#include <ctime>     // Agregado para obtener la fecha y hora actual

using namespace std;

class Server {
  private: string rolUsuario; // Declaración del rol del usuario
  public: WSADATA WSAData;
  SOCKET server;
  SOCKET client; // Cambiado a SOCKET en lugar de SOCKET client;
  SOCKADDR_IN serverAddr;
  SOCKADDR_IN clientAddr;
  char buffer[1024];

  Server() {
    WSAStartup(MAKEWORD(2, 0), & WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5005);

    bind(server, (SOCKADDR * ) & serverAddr, sizeof(serverAddr));
    listen(server, 0);

    cout << "Escuchando para conexiones entrantes en el puerto 5005." << endl;
    RegistrarEvento("=======Inicia Servidor=======");

    // Aceptar la conexión del cliente antes de entrar en el bucle principal
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
    RegistrarEvento("Cierre de sesion."); // Registrar cierre de sesión
    cout << "Socket cerrado, cliente desconectado." << endl;
    // Aceptar una nueva conexión antes de salir de esta función
    int clientAddrSize = sizeof(clientAddr);
    if ((client = accept(server, (SOCKADDR * ) & clientAddr, & clientAddrSize)) != INVALID_SOCKET) {
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
  void InsertarNuevaTraduccion() {
    Enviar("Ingrese una nueva traducción en el formato palabraEnInglés:traducciónEnEspañol:");

    while (true) {
        string nuevaTraduccion = Recibir();
        if (nuevaTraduccion.empty()) break; // El cliente se ha desconectado, sale del bucle interno

        if (nuevaTraduccion == "/salir") {
            Enviar("Has vuelto al menu principal.");
            break; // El usuario escribió /salir, sale del bucle interno
        }

        // Dividir la nueva traducción en palabra en inglés y traducción en español
        size_t pos = nuevaTraduccion.find(":");
        if (pos == string::npos) {
            Enviar("Formato de traduccion incorrecto. Use palabraEnIngles:traduccionEnEspanol.");
            continue; // Volver a solicitar una nueva traducción
        }

        string palabraIngles = nuevaTraduccion.substr(0, pos);

        // Verificar si ya existe una traducción para la palabra en inglés
        string traduccionExistente = BuscarTraduccionEnArchivo(palabraIngles);

        if (traduccionExistente !="No fue posible encontrar la traducci�n para:" + palabraIngles) {
            Enviar("Ya existe una traduccion para " + palabraIngles + ": " + traduccionExistente);
        } else {
            // Agregar la nueva traducción al archivo de traducciones
            ofstream archivo("traducciones.txt", ios::app);
            if (archivo.is_open()) {
                string traduccionEspanol = nuevaTraduccion.substr(pos + 1);
                archivo << palabraIngles << ":" << traduccionEspanol << endl;
                archivo.close();
                Enviar("Nueva traduccion agregada con exito.");
            } else {
                Enviar("Error al agregar la nueva traducción.");
            }
        }
    }
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
      RegistrarEvento("Inicio de sesion – usuario: " + usuario); // Registrar inicio de sesión
      return true; // Usuario aceptado, retorno true
    } else {
      BloquearUsuario(usuario); // Sumo intentos y si tiene 3 o mas lo baneo
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
          if (usuarioArchivo == usuario) {
            // Verificar si el usuario está bloqueado
            if (intentos >= 3) {
              archivo.close();
              return false;
            }

            if (contrasenaArchivo == contrasena) {
              // Las credenciales son válidas
              archivo.close();
              Enviar("Autenticación exitosa. ¡Bienvenido!");
              rolUsuario = rol; // Almacenar el rol del usuario actual
              return true;
            }
          }
        }
      }
      archivo.close();
    }

    return false; // Las credenciales no son válidas o no se encontraron en el archivo
  }

  void BloquearUsuario(string usuario) {
    ifstream archivo("credenciales.txt");
    ofstream archivoTemp("credenciales_temp.txt");
    if (archivo.is_open() && archivoTemp.is_open()) {
      string linea;
      while (getline(archivo, linea)) {
        istringstream iss(linea);
        string usuarioArchivo, contrasenaArchivo, rol;
        int intentos;

        if (getline(iss, usuarioArchivo, '|') &&
          getline(iss, contrasenaArchivo, '|') &&
          getline(iss, rol, '|') &&
          (iss >> intentos)) {
          if (usuarioArchivo == usuario) {
            // Incrementar el contador de intentos fallidos
            intentos++;
            if (intentos >= 3) {
              Enviar("Se ha superado la cantidad maxima de intentos, la cuenta " + usuario + " ha sido bloqueada.");
            } else {
              Enviar("Datos de usuario incorrectos. La conexión se cerrará.");
            }
          }
          archivoTemp << usuarioArchivo << "|" << contrasenaArchivo << "|" << rol << "|" << intentos << endl;
        } else {
          archivoTemp << linea << endl;
        }
      }
      archivo.close();
      archivoTemp.close();
      remove("credenciales.txt");
      rename("credenciales_temp.txt", "credenciales.txt");
    }
  }

  //!ADMIN USUARIOS
  void SubmenuUsuarios() {
    Enviar("\nSubmenu Usuarios\n1. Alta\n2. Desbloqueo\nEscriba /salir para volver al menu principal");
    while (true) {
      string opcion = Recibir();

      if (opcion.empty()) break; // El cliente se ha desconectado, sale del bucle

      if (opcion == "1") {
        DarAltaUsuario();
      } else if (opcion == "2") {
        ListarUsuariosBloqueados();
      } else if (opcion == "/salir") {
        Enviar("Has vuelto al menu principal.");
        break;
      } else {
        Enviar("Inserte una opción válida (1, 2 o /salir)");
      }
    }
  }

  void DarAltaUsuario() {
    Enviar("Ingrese nuevo usuario (Nombre:Contraseña):");
    string nuevoUsuario = Recibir();

    // Verificar si el usuario ingresó datos válidos
    if (nuevoUsuario.empty() || nuevoUsuario.find(':') == std::string::npos) {
      Enviar("Error al dar de alta el nuevo usuario: datos incompletos o formato incorrecto");
      return;
    }

    // Reemplazar los ":" con "|" en la cadena de entrada
    replace(nuevoUsuario.begin(), nuevoUsuario.end(), ':', '|');

    // Separar el nombre de usuario y la contraseña
    size_t separadorPos = nuevoUsuario.find('|');
    if (separadorPos == std::string::npos) {
      Enviar("Error al dar de alta el nuevo usuario: formato incorrecto (Nombre:Contraseña)");
      return;
    }

    string nombreUsuario = nuevoUsuario.substr(0, separadorPos);
    string contrasena = nuevoUsuario.substr(separadorPos + 1);

    // Verificar si el nombre de usuario y la contraseña no son vacíos
    if (nombreUsuario.empty() || contrasena.empty()) {
      Enviar("Error al dar de alta el nuevo usuario: nombre de usuario o contraseña vacíos");
      return;
    }

    // Convertir el nombre de usuario a minúsculas
    nombreUsuario = ConvertirAMinusculas(nombreUsuario);

    // Verificar si el usuario ya existe
    if (UsuarioExiste(nombreUsuario)) {
      Enviar("Error al dar de alta el nuevo usuario: usuario existente");
      return;
    }

    // Agregar el nuevo usuario al archivo de credenciales
    ofstream archivoCredenciales("credenciales.txt", std::ios::app);
    if (!archivoCredenciales.is_open()) {
      Enviar("Error al abrir el archivo de credenciales para agregar el nuevo usuario");
      return;
    }

    // Establecer el campo intentos a 0 y el rol a CONSULTA
    archivoCredenciales << nombreUsuario << "|" << contrasena << "|CONSULTA|0" << endl;
    archivoCredenciales.close();

    Enviar(nombreUsuario + " dado de alta correctamente");
  }

  bool UsuarioExiste(const string & nombreUsuario) {
    ifstream archivoCredenciales("credenciales.txt");
    if (archivoCredenciales.is_open()) {
      string linea;
      while (getline(archivoCredenciales, linea)) {
        istringstream iss(linea);
        string usuarioArchivo;

        if (getline(iss, usuarioArchivo, '|')) {
          // Convertir el nombre de usuario almacenado a minúsculas y comparar
          usuarioArchivo = ConvertirAMinusculas(usuarioArchivo);
          if (usuarioArchivo == nombreUsuario) {
            archivoCredenciales.close();
            return true; // El usuario ya existe
          }
        }
      }
      archivoCredenciales.close();
    }
    return false; // El usuario no existe
  }

  void ListarUsuariosBloqueados() {
    ifstream archivo("credenciales.txt");
    if (!archivo.is_open()) {
        // Error al abrir el archivo.
        return;
    }

    // Crea un vector para almacenar los usuarios bloqueados.
    vector<string> usuariosBloqueados;

    // Lee el archivo de credenciales.
    string linea;
    while (getline(archivo, linea)) {
        // Parsea la línea del archivo.
        istringstream iss(linea);
        string usuarioArchivo, contrasenaArchivo, rol;
        int intentos;

        if (getline(iss, usuarioArchivo, '|') &&
            getline(iss, contrasenaArchivo, '|') &&
            getline(iss, rol, '|') &&
            (iss >> intentos)) {
            // Si el número de intentos es mayor o igual a 3, el usuario está bloqueado.
            if (intentos >= 3) {
                usuariosBloqueados.push_back(usuarioArchivo);
            }
        }
    }

    archivo.close(); // Cerrar el archivo después de la lectura.

    string mensaje = ""; // Cadena para almacenar el mensaje completo

    if (!usuariosBloqueados.empty()) {
        mensaje += "\nUsuarios bloqueados:\n";
        for (const string& usuarioBloqueado : usuariosBloqueados) {
            mensaje += usuarioBloqueado + "\n";
        }
        mensaje += "Escriba /salir para volver al submenu de usuarios o ingrese el nombre de usuario que desea desbloquear:";
        Enviar(mensaje); // Enviar el mensaje completo
        string respuesta = Recibir();

        if (respuesta == "/salir") {
            Enviar("Has vuelto al submenu de usuarios.\nSubmenu Usuarios\n1. Alta\n2. Desbloqueo\nEscriba /salir para volver al menu principal");
        } else {
            auto it = find(usuariosBloqueados.begin(), usuariosBloqueados.end(), respuesta);
            if (it != usuariosBloqueados.end()) {
                // Restablecer intentos a 0
                RestablecerIntentos(respuesta);
                Enviar(respuesta + " desbloqueado correctamente");
            } else {
                Enviar("El usuario " + respuesta + " no se encuentra bloqueado.");
            }
        }
    } else {
        Enviar("No se encontraron usuarios bloqueados. Escriba /salir para volver al submenu de usuarios:");
    }
}

void RestablecerIntentos(const string& nombreUsuario) {
    // Abre el archivo de credenciales.
    ifstream archivoEntrada("credenciales.txt");
    if (!archivoEntrada.is_open()) {
        // Error al abrir el archivo.
        return;
    }

    // Crea un archivo temporal para almacenar las credenciales actualizadas.
    ofstream archivoSalida("credenciales_temp.txt");

    // Lee el archivo de credenciales.
    string linea;
    while (getline(archivoEntrada, linea)) {
        // Parsea la línea del archivo.
        istringstream iss(linea);
        string usuarioArchivo, contrasenaArchivo, rol;
        int intentos;

        if (getline(iss, usuarioArchivo, '|') &&
            getline(iss, contrasenaArchivo, '|') &&
            getline(iss, rol, '|') &&
            (iss >> intentos)) {
            // Si el nombre de usuario coincide, establece el número de intentos a 0.
            if (usuarioArchivo == nombreUsuario) {
                intentos = 0;
            }

            // Escribe la línea actualizada en el archivo temporal.
            archivoSalida << usuarioArchivo << '|' << contrasenaArchivo << '|' << rol << '|' << intentos << endl;
        }
    }

    // Cierra los archivos.
    archivoEntrada.close();
    archivoSalida.close();

    // Elimina el archivo de credenciales original y renombra el archivo temporal.
    remove("credenciales.txt");
    rename("credenciales_temp.txt", "credenciales.txt");
}


  //!MENU
  void MenuPrincipal() {
    while (true) {
      if (rolUsuario == "ADMIN") {
        // Menú para el rol de administrador
        Enviar("\nMenu Principal (Rol: ADMIN):\n1. Traductor\n2. Insertar Nueva Traduccion\n3. Submenu Usuarios\n4. Ver registro de actividades\n5. Salir");
        string opcion = Recibir();
        if (opcion.empty()) break; // El cliente se ha desconectado, sale del bucle
        if (opcion == "1") Traductor();
        if (opcion == "2") InsertarNuevaTraduccion();
        if (opcion == "3") SubmenuUsuarios();
        if (opcion == "4") EnviarRegistroActividades();
        if (opcion == "5") break;
        else Enviar("Inserte una opción válida (1-5)");
      }

      if (rolUsuario == "CONSULTA") {
        // Menú para el rol de consulta
        Enviar("\nMenu Principal (Rol: CONSULTA):\n1. Traductor\n2. Salir");
        string opcion = Recibir();
        if (opcion.empty()) break; // El cliente se ha desconectado, sale del bucle
        if (opcion == "1") Traductor();
        if (opcion == "2") break;
        else Enviar("Inserte una opcion valida (1-2)");
      }

    }

  }
  //! EVENTOS
  void RegistrarEvento(const string &evento) {
    ofstream logFile("server.log", ios::app);
    if (logFile.is_open()) {
        time_t now = time(0);
        struct tm tstruct;
        char buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d_%H:%M:%S", &tstruct);

        logFile << buf << ": " << evento << endl;
        logFile.close();
    } else {
        cout << "Error al abrir el archivo de registro." << endl;
    }
}
// Modifica la función EnviarRegistroActividades en la clase Server
void EnviarRegistroActividades() {
    ifstream logFile("server.log");
    if (!logFile.is_open()) {
        Enviar("Error: No se pudo abrir el archivo de registro de actividades.");
        return;
    }

    string linea;
    while (getline(logFile, linea)) {
        Enviar(linea); // Envía cada línea del registro como un mensaje separado
    }

    logFile.close();
    // Envía un mensaje especial para indicar que se han enviado todas las líneas
    Enviar("FIN_DEL_REGISTRO");
}


};

int main() {
  Server * Servidor = new Server();
  while (true) {
    if (Servidor -> AceptarCliente()) {
      Servidor -> MenuPrincipal();
    }
  }
  delete Servidor;
  return 0;
}
