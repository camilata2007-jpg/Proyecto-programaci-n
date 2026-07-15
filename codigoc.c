#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>   //NUEVO: necesaria para obtener la fecha del sistema

#define ARCH_PRODUCTOS  "productos.dat"
#define ARCH_CLIENTES   "clientes.dat"
#define ARCH_VENTAS     "ventas.dat"
#define ARCH_INVENTARIO "inventario.dat"
#define ARCH_CAJA       "caja.dat"
#define ARCH_CIERRES    "cierres_caja.txt"

#define TAM_NOMBRE 50
#define TAM_CEDULA 11
#define TAM_FECHA  11


#define ROL_ADMIN      1
#define ROL_CAJERO     2
#define ROL_INVENTARIO 3
#define ROL_CLIENTE    4


#define CLAVE_ADMIN      "admin2026"
#define CLAVE_CAJERO     "cajero2026"
#define CLAVE_INVENTARIO "inventario2026"


typedef struct Producto {
    int   codigo;
    char  nombre[TAM_NOMBRE];
    float precio;
    int   stock;
    int   stockMinimo;
    int   iva;
    struct Producto *siguiente;
} Producto;


typedef struct Cliente {
    char cedula[TAM_CEDULA];
    char nombre[TAM_NOMBRE];
    struct Cliente *siguiente;
} Cliente;


typedef struct Venta {
    int   id;
    char  fecha[TAM_FECHA];
    char  cedulaCliente[TAM_CEDULA];
    char  nombreCliente[TAM_NOMBRE];
    int   codigoProducto;
    char  nombreProducto[TAM_NOMBRE];
    int   cantidad;
    float precioUnitario;
    float subtotal;
    float descuento;
    float valorIVA;
    float total;
    struct Venta *siguiente;
} Venta;


typedef struct Movimiento {
    char fecha[TAM_FECHA];
    char tipo[16];
    int  codigoProducto;
    char nombreProducto[TAM_NOMBRE];
    int  cantidad;
    int  stockResultante;
} Movimiento;


typedef struct EstadoCaja {
    int   abierta;
    float montoApertura;
    int   idVentaInicio;
} EstadoCaja;


typedef struct ItemVendido {
    int   codigo;
    char  nombre[TAM_NOMBRE];
    int   unidades;
    float monto;
} ItemVendido;


typedef struct ClienteRank {
    char  cedula[TAM_CEDULA];
    char  nombre[TAM_NOMBRE];
    float monto;
    int   compras;
} ClienteRank;


Producto  *listaProductos = NULL;
Cliente   *listaClientes  = NULL;
Venta     *listaVentas    = NULL;
EstadoCaja caja           = {0, 0.0f, 0};
Cliente   *clienteActual  = NULL;
int siguienteIdVenta      = 1;


void limpiarBuffer(void) {
    int caracter;
    while ((caracter = getchar()) != '\n' && caracter != EOF);
}

//ingresa la direccion de mensaje y direccion de destino y el tamano de la cadena
void leerCadena(const char *mensaje, char *destino, int tamanio) {
    printf("%s", mensaje);
    //almacena en destino la cadena ingresada por el usuario, si no hay mas entrada, cierra el programa
    if (fgets(destino, tamanio, stdin) == NULL) {
        printf("\n Entrada finalizada. Cerrando el sistema de forma segura.\n");
        //cierra el programa con un estado de salida exitoso
        exit(EXIT_SUCCESS);
    }
    //size_t es un tipo de dato que representa un entero sin signo
    size_t longitud = strlen(destino);
    if (longitud > 0 && destino[longitud - 1] == '\n'){
        //eliminar enter del final
        destino[longitud - 1] = '\0';
    }else{
        limpiarBuffer(); }
}


int leerEntero(const char *mensaje) {
    char linea[64];
    int valor;
    while (1) {
        leerCadena(mensaje, linea, sizeof(linea));
        if (sscanf(linea, "%d", &valor) == 1) return valor;
        printf(" Entrada invalida. Ingrese un numero entero.\n");
    }
}

//pasa por funcion leer cadena y leer entero, y valida que el entero sea positivo
int leerEnteroPositivo(const char *mensaje) {
    int valor;
    while (1) {
        valor = leerEntero(mensaje);
        if (valor > 0) return valor;
        printf("El valor debe ser MAYOR a cero (no se aceptan negativos ni cero).\n");
    }
}


int leerEnteroEnRango(const char *mensaje, int minimo, int maximo) {
    int valor;
    while (1) {
        valor = leerEntero(mensaje);
        if (valor >= minimo && valor <= maximo) return valor;
        printf("Opcion fuera de rango (%d - %d).\n", minimo, maximo);
    }
}

float leerFlotante(const char *mensaje) {
    char linea[64];
    float valor;
    while (1) {
        leerCadena(mensaje, linea, sizeof(linea));
        if (sscanf(linea, "%f", &valor) == 1) return valor;
        printf("  >> Entrada invalida. Ingrese un numero.\n");
    }
}


float leerDineroPositivo(const char *mensaje) {
    float valor;
    while (1) {
        valor = leerFlotante(mensaje);
        if (valor > 0.0f) return valor;
        printf("Monto invalido: el dinero debe ser MAYOR a cero.\n");
    }
}


float redondear2(float valor) {
    // redondea a 2 decimales: multiplicar por 100, sumar 0.5, truncar a entero, dividir entre 100
    return (float)((long)(valor * 100.0f + 0.5f)) / 100.0f;
}


int esFechaValida(const char *fecha) {
    int anio, mes, dia;
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') return 0;
    //scanf devuelve 1 por cada variable que logre entrar con exito
    if (sscanf(fecha, "%4d-%2d-%2d", &anio, &mes, &dia) != 3) return 0;
    if (anio < 2000 || anio > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > 31) return 0;
    return 1;
}

//se usa SOLO en los reportes por rango: ahi el usuario elige que fechas filtrar
void leerFecha(const char *mensaje, char *destino) {
    while (1) {
        leerCadena(mensaje, destino, TAM_FECHA);
        if (esFechaValida(destino)) return;
        printf("Fecha invalida. Use el formato AAAA-MM-DD.\n");
    }
}

//NUEVO: obtiene la fecha actual DEL SISTEMA en formato AAAA-MM-DD.
//El usuario ya NO ingresa la fecha en compras, ajustes ni ventas.
void fechaHoy(char *destino) {
    //time(NULL) devuelve los segundos transcurridos desde 1970 (reloj del sistema)
    time_t segundosActuales = time(NULL);
    //localtime convierte esos segundos a anio, mes, dia... en hora local
    struct tm *tiempoLocal = localtime(&segundosActuales);
    //strftime escribe la fecha con el formato AAAA-MM-DD en destino
    strftime(destino, TAM_FECHA, "%Y-%m-%d", tiempoLocal);
}

int validarCedulaEcuatoriana(const char *cedula) {
    int posicion, suma = 0;
    int provincia, tercerDigito, digitoVerificador, decenaSuperior;
    if (strlen(cedula) != 10) return 0;
    for (posicion = 0; posicion < 10; posicion++)
    //implementacion de la logica de validacion de cedula ecuatoriana modulo 10
    //Los 9 primeros digitos se multiplican alternadamente por 2 y 1.
    //A los resultados mayores a 9 se les resta 9. La suma total se resta del numero superior multiplo de 10
    // mas cercano para obtener el verificador.
        if (cedula[posicion] < '0' || cedula[posicion] > '9') return 0;
    provincia = (cedula[0] - '0') * 10 + (cedula[1] - '0');
    if (!((provincia >= 1 && provincia <= 24) || provincia == 30)) return 0;
    tercerDigito = cedula[2] - '0';
    if (tercerDigito >= 6) return 0;
    for (posicion = 0; posicion < 9; posicion++) {
        int digito = cedula[posicion] - '0';
        int producto = (posicion % 2 == 0) ? digito * 2 : digito * 1;
        if (producto > 9) producto -= 9;
        suma += producto;
    }
    decenaSuperior = ((suma + 9) / 10) * 10;
    digitoVerificador = decenaSuperior - suma;
    if (digitoVerificador == 10) digitoVerificador = 0;
    return digitoVerificador == (cedula[9] - '0');
}

int leerIVA(void) {
    int porcentaje;
    while (1) {
        porcentaje = leerEntero("  IVA del producto en % (solo 0, 12 o 15): ");
        if (porcentaje == 0 || porcentaje == 12 || porcentaje == 15)
            return porcentaje;
        printf("IVA invalido: los unicos valores permitidos son 0, 12 y 15.\n");
    }
}

const char *nombreRol(int rol) {
    if (rol == ROL_ADMIN)   return "ADMINISTRADOR";
    if (rol == ROL_CAJERO)  return "CAJERO";
    if (rol == ROL_CLIENTE) return "CLIENTE";
    return "ENCARGADO DE INVENTARIO";
}

void accesoDenegado(void) {
    printf("\n Su rol no tiene permisos para esta operacion.\n");
}

int rolTieneAcceso(int rol, int modulo) {
    if (rol == ROL_ADMIN) return 1;
    if (rol == ROL_CAJERO)
        {return modulo == 1 || modulo == 3 || modulo == 4 || modulo == 6;}
    return modulo == 1 || modulo == 2 || modulo == 5;
}

void guardarProductos(void) {
    FILE *archivo = fopen(ARCH_PRODUCTOS, "wb");    // "wb": escribir binario
    if (archivo == NULL) { printf("Error al abrir %s\n", ARCH_PRODUCTOS); return; }
    //recorre la lista de productos y escribe cada nodo en el archivo binario
    Producto *producto = listaProductos;
    while (producto != NULL) {
        //escribe el producto completo en el archivo binario
        fwrite(producto, sizeof(Producto), 1, archivo);
        //avanza al siguiente producto en la lista
        producto = producto->siguiente;
    }
    fclose(archivo);
}


void cargarProductos(void) {
    FILE *archivo = fopen(ARCH_PRODUCTOS, "rb");    // "rb": leer binario
    if (archivo == NULL) return;
    Producto leido, *ultimo = NULL;
    while (fread(&leido, sizeof(Producto), 1, archivo) == 1) {  // mientras haya registros
        // crea un nuevo nodo de producto en memoria dinamica
        Producto *nuevo = (Producto *)malloc(sizeof(Producto));
        //si no hay memoria suficiente, muestra un mensaje y sale del bucle
        if (nuevo == NULL) { printf("  >> Sin memoria.\n"); break; }
        //copia los datos leidos del archivo al nuevo nodo
        *nuevo = leido;
        //inicializa el puntero siguiente del nuevo nodo a NULL
        nuevo->siguiente = NULL;
        //si la lista de productos esta vacia, el nuevo nodo se convierte en la cabeza de la lista
        if (listaProductos == NULL){ listaProductos = nuevo;}
        //si la lista no esta vacia, enlaza el nuevo nodo al final de la lista
        else {ultimo->siguiente = nuevo;}
        //actualiza el puntero ultimo para que apunte al nuevo nodo
        ultimo = nuevo;
    }
    fclose(archivo);
}


Producto *buscarProductoSecuencial(int codigo) {
    Producto *producto = listaProductos;
    while (producto != NULL) {
        //compara el codigo del producto actual con el codigo buscado
        if (producto->codigo == codigo) return producto;
        //avanza al siguiente producto en la lista
        producto = producto->siguiente;
    }
    return NULL;
}

// total de productos en la lista dinamica
int contarProductos(void) {
    int cantidad = 0;
    Producto *producto;
    for (producto = listaProductos; producto != NULL; producto = producto->siguiente)
        cantidad++;
    return cantidad;
}

Producto *buscarProductoBinaria(int codigoBuscado) {
    int totalProductos = contarProductos();
    int posicion = 0;
    if (totalProductos == 0) return NULL;
    Producto **arreglo = (Producto **)malloc(totalProductos * sizeof(Producto *));
    if (arreglo == NULL) return NULL;
    Producto *producto;
    //copia los punteros de la lista dinamica a un arreglo de punteros
    for (producto = listaProductos; producto != NULL; producto = producto->siguiente)
    //copia lista -> arreglo
        arreglo[posicion++] = producto;
    //Algoritmo de ordenamiento BURBUJA para ordenar el arreglo de punteros a productos por codigo
    for (int pasada = 0; pasada < totalProductos - 1; pasada++)
        for (int indice = 0; indice < totalProductos - 1 - pasada; indice++)
            if (arreglo[indice]->codigo > arreglo[indice + 1]->codigo) {
                Producto *temporal = arreglo[indice];
                arreglo[indice] = arreglo[indice + 1];
                arreglo[indice + 1] = temporal;
            }
    // Algoritmo de busqueda binaria para encontrar el producto con el codigo buscado
    int izquierda = 0, derecha = totalProductos - 1;
    Producto *resultado = NULL;
    while (izquierda <= derecha) {
        int centro = (izquierda + derecha) / 2;
        if (arreglo[centro]->codigo == codigoBuscado) {
            resultado = arreglo[centro];
            break;
        }
        else if (arreglo[centro]->codigo < codigoBuscado)
            izquierda = centro + 1;
        else
            derecha = centro - 1;
    }
    free(arreglo);
    return resultado;
}


void registrarProducto(void) {
    Producto *nuevo = (Producto *)malloc(sizeof(Producto));
    if (nuevo == NULL) { printf("  >> Sin memoria.\n"); return; }
    printf("\n--- REGISTRO DE PRODUCTO ---\n");
    while (1) {
        nuevo->codigo = leerEnteroPositivo("  Codigo del producto: ");
        //verifica si el producto con el codigo ingresado ya existe en la lista de productos
        if (buscarProductoSecuencial(nuevo->codigo) == NULL) break;
        //si el producto ya existe, muestra un mensaje de error y solicita otro codigo
        printf("Ya existe un producto con ese codigo.\n");
    }
    leerCadena("  Nombre: ", nuevo->nombre, TAM_NOMBRE);
    while (strlen(nuevo->nombre) == 0)
        leerCadena("  Nombre: ", nuevo->nombre, TAM_NOMBRE);
    nuevo->precio      = redondear2(leerDineroPositivo("  Precio unitario (USD): "));
    nuevo->stock       = leerEnteroPositivo("  Stock inicial: ");
    nuevo->stockMinimo = leerEnteroPositivo("  Stock minimo (alerta): ");
    nuevo->iva         = leerIVA();
    nuevo->siguiente = listaProductos;
    //inserta el nuevo producto al inicio de la lista de productos
    listaProductos = nuevo;
    guardarProductos();
    printf("  >> Producto registrado y guardado en archivo binario.\n");
}

/* EDICION / ACTUALIZACION persistente de producto */
void editarProducto(void) {
    int codigo = leerEnteroPositivo("  Codigo del producto a editar: ");
    Producto *producto = buscarProductoBinaria(codigo);
    if (producto == NULL) { printf(" Producto no encontrado.\n"); return; }
    printf("  Editando: %s (precio %.2f, stock %d, IVA %d%%)\n",
           producto->nombre, producto->precio, producto->stock, producto->iva);
    char nuevoNombre[TAM_NOMBRE];
    leerCadena("  Nuevo nombre (ENTER = mantener el actual): ", nuevoNombre, TAM_NOMBRE);
    if (strlen(nuevoNombre) > 0) strcpy(producto->nombre, nuevoNombre);
    producto->precio      = redondear2(leerDineroPositivo("  Nuevo precio (USD): "));
    producto->stockMinimo = leerEnteroPositivo("  Nuevo stock minimo: ");
    producto->iva         = leerIVA();
    guardarProductos();
    printf(" Producto actualizado en el archivo binario.\n");
}


void eliminarProducto(void) {
    int codigo = leerEnteroPositivo("  Codigo del producto a eliminar: ");
    Producto *actual = listaProductos, *anterior = NULL;
    while (actual != NULL && actual->codigo != codigo) {      /* [R10] secuencial */
        anterior = actual;
        actual = actual->siguiente;
    }
    if (actual == NULL) { printf(" Producto no encontrado.\n"); return; }
    //si el producto a eliminar es el primero de la lista, actualiza la cabeza de la lista
    if (anterior == NULL) listaProductos = actual->siguiente;
    //si el producto a eliminar no es el primero, enlaza el nodo anterior con el siguiente del actual
    else anterior->siguiente = actual->siguiente;
    free(actual);
    //guarda la lista de productos actualizada en el archivo binario
    guardarProductos();
    printf(" Producto eliminado del sistema y del archivo.\n");
}


void listarProductos(void) {
    if (listaProductos == NULL) { printf(" No hay productos registrados.\n"); return; }
    printf("\n  %-8s %-25s %10s %8s %8s %6s\n", "Codigo", "Nombre", "Precio", "Stock", "Minimo", "IVA");
    printf("  ---------------------------------------------------------------------\n");
    Producto *producto;
    //recorre la lista de productos y muestra la informacion de cada producto en formato tabular
    for (producto = listaProductos; producto != NULL; producto = producto->siguiente)
        printf("  %-8d %-25s %10.2f %8d %8d %5d%%\n",
               producto->codigo, producto->nombre, producto->precio,
               producto->stock, producto->stockMinimo, producto->iva);
}


void respaldarMovimiento(Movimiento movimiento) {
    //abre el archivo de inventario en modo "append" para agregar al final del archivo
    FILE *archivo = fopen(ARCH_INVENTARIO, "ab");
    if (archivo == NULL) { printf(" Error al abrir %s\n", ARCH_INVENTARIO); return; }
    //escribe el movimiento completo en el archivo binario
    fwrite(&movimiento, sizeof(Movimiento), 1, archivo);
    fclose(archivo);
}


void ajustarStock(int *stock, int cantidad) {
    *stock = *stock + cantidad;
}

void registrarMovimientoInventario(Producto *producto, const char *tipo, int cantidad, const char *fecha)
 {
    Movimiento movimiento;
    //copia la fecha y el tipo de movimiento a la estructura Movimiento
    //strcpy copia la cadena de caracteres de fecha a movimiento.fecha
    strcpy(movimiento.fecha, fecha);
    strncpy(movimiento.tipo, tipo, sizeof(movimiento.tipo) - 1);
    movimiento.tipo[sizeof(movimiento.tipo) - 1] = '\0';
    movimiento.codigoProducto = producto->codigo;
    // copia el nombre del producto al movimiento, asegurando que no se desborde el tamano del campo
    strncpy(movimiento.nombreProducto, producto->nombre, TAM_NOMBRE - 1);
    movimiento.nombreProducto[TAM_NOMBRE - 1] = '\0';
    movimiento.cantidad = cantidad;
    // ajusta el stock del producto usando la direccion de memoria del stock
    //se usa & porque se pasa la direccion de memoria del stock del producto a la funcion ajustarStock
    ajustarStock(&producto->stock, cantidad);
    movimiento.stockResultante = producto->stock;
    respaldarMovimiento(movimiento);
    //funcion que guarda la lista de productos actualizada en el archivo binario, ES GLOBAL, no necesita parametros
    guardarProductos();
}

// registra una compra de un producto, actualiza el stock y guarda el movimiento en el archivo binario
void registrarCompra(void) {
    printf("\n--- REGISTRO DE COMPRA---\n");
    int codigo = leerEnteroPositivo("  Codigo del producto: ");
    Producto *producto = buscarProductoSecuencial(codigo);
    if (producto == NULL) { printf(" Producto no encontrado.\n"); return; }
    int cantidad = leerEnteroPositivo(" Cantidad comprada: ");
    //MODIFICADO: la fecha ya no la ingresa el usuario, la da el SISTEMA
    char fecha[TAM_FECHA];
    fechaHoy(fecha);
    printf("  Fecha de la compra (sistema): %s\n", fecha);
    registrarMovimientoInventario(producto, "COMPRA", cantidad, fecha);
    // muestra un mensaje indicando que la compra fue registrada y el nuevo stock del producto
    printf("Compra registrada. Nuevo stock de '%s': %d\n",
           producto->nombre, producto->stock);
}
// registra un ajuste de stock (entrada o salida) de un producto, actualiza el stock y guarda el movimiento en el archivo binario
void registrarAjuste(void) {
    printf("\n--- AJUSTE DE STOCK ---\n");
    int codigo = leerEnteroPositivo("  Codigo del producto: ");
    Producto *producto = buscarProductoSecuencial(codigo);
    if (producto == NULL) { printf(" Producto no encontrado.\n"); return; }
    printf("  Stock actual de '%s': %d\n", producto->nombre, producto->stock);
    int cantidad;
    while (1) {
        cantidad = leerEntero("  Cantidad de ajuste (+entrada / -salida): ");
        if (cantidad == 0) { printf(" El ajuste no puede ser cero.\n"); continue; }
        if (producto->stock + cantidad < 0) {
            //si el ajuste dejaria el stock en negativo, muestra un mensaje de error y solicita otro ajuste
            printf("El ajuste dejaria el stock en negativo (stock actual: %d).\n",
                   producto->stock);
            continue;
            //si el ajuste es valido, sale del bucle
        }
        break;
    }
    //MODIFICADO: la fecha ya no la ingresa el usuario, la da el SISTEMA
    char fecha[TAM_FECHA];
    fechaHoy(fecha);
    printf("  Fecha del ajuste (sistema): %s\n", fecha);
    registrarMovimientoInventario(producto, "AJUSTE", cantidad, fecha);
    printf(" Ajuste aplicado. Nuevo stock: %d\n", producto->stock);
}

// muestra el historial de movimientos de inventario (compras y ajustes) desde el archivo binario
void consultarHistoricoInventario(void) {
    FILE *archivo = fopen(ARCH_INVENTARIO, "rb");
    //si no se puede abrir el archivo, muestra un mensaje indicando que no hay movimientos registrados y retorna
    if (archivo == NULL) { printf("No existen movimientos registrados.\n"); return; }
    Movimiento movimiento;
    int totalMovimientos = 0;
    printf("\n  %-8s %-8s %-25s %10s %8s\n",
           "Tipo", "CodProd", "Producto", "Cantidad", "Stock");
    printf("  ----------------------------------------------------------------\n");
    //lee cada movimiento del archivo binario y lo muestra en formato tabular
    while (fread(&movimiento, sizeof(Movimiento), 1, archivo) == 1) {
        printf("  %-8s %-8d %-25s %10d %8d\n",
               movimiento.tipo, movimiento.codigoProducto,
               movimiento.nombreProducto, movimiento.cantidad,
               movimiento.stockResultante);
               //incrementa el contador de movimientos leidos
        totalMovimientos++;
    }
    fclose(archivo);
    printf("  Total de movimientos: %d\n", totalMovimientos);
}

// guarda la lista de clientes en un archivo binario, escribiendo cada nodo de la lista en el archivo
void guardarClientes(void) {
    FILE *archivo = fopen(ARCH_CLIENTES, "wb");
    if (archivo == NULL) { printf(" Error al abrir %s\n", ARCH_CLIENTES); return; }
    Cliente *cliente;
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
        fwrite(cliente, sizeof(Cliente), 1, archivo);
    fclose(archivo);
}

// carga la lista de clientes desde un archivo binario, leyendo cada registro y creando un nodo en memoria dinamica
void cargarClientes(void) {
    FILE *archivo = fopen(ARCH_CLIENTES, "rb");        // "rb": leer binario
    if (archivo == NULL) return;
    Cliente leido, *ultimo = NULL;
    //lee cada registro del archivo binario y crea un nuevo nodo de cliente en memoria dinamica
    while (fread(&leido, sizeof(Cliente), 1, archivo) == 1) {
        Cliente *nuevo = (Cliente *)malloc(sizeof(Cliente));
        if (nuevo == NULL) break;
        *nuevo = leido;
        nuevo->siguiente = NULL;
        if (listaClientes == NULL) listaClientes = nuevo;
        else ultimo->siguiente = nuevo;
        ultimo = nuevo;
    }
    fclose(archivo);
}

void mostrarCliente(Cliente cliente) {
    printf("  Cedula: %s | Nombre: %s\n", cliente.cedula, cliente.nombre);
}

int cedulaYaRegistrada(char cedula[TAM_CEDULA]) {
    Cliente *cliente;
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
    //compara la cedula del cliente actual con la cedula buscada
        if (strcmp(cliente->cedula, cedula) == 0) return 1;
    return 0;
}

// busca un cliente en la lista de clientes por su cedula, devolviendo un puntero al cliente si se encuentra, o NULL si no se encuentra
Cliente *buscarClientePorCedula(const char *cedula) {
    Cliente *cliente;
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
        if (strcmp(cliente->cedula, cedula) == 0) return cliente;
    return NULL;
}
// registra un nuevo cliente, validando la cedula y asegurando que no este ya registrada, y guarda el cliente en la lista y en el archivo binario
Cliente *registrarCliente(void) {
    char cedula[TAM_CEDULA];
    printf("\n--- REGISTRO DE CLIENTE ---\n");
    while (1) {
        leerCadena("  Cedula ecuatoriana: ", cedula, TAM_CEDULA);
        if (!validarCedulaEcuatoriana(cedula)) {
            printf(" Cedula ecuatoriana INVALIDA.\n");
            continue;
        }
        if (cedulaYaRegistrada(cedula)) {
            printf(" Esa cedula ya esta registrada.\n");
            continue;
        }
        break;
    }
    //crea un nuevo nodo de cliente en memoria dinamica
    Cliente *nuevo = (Cliente *)malloc(sizeof(Cliente));
    if (nuevo == NULL) { printf(" Sin memoria.\n"); return NULL; }
    strcpy(nuevo->cedula, cedula);
    leerCadena("  Nombre completo: ", nuevo->nombre, TAM_NOMBRE);
    //si el nombre ingresado es una cadena vacia, solicita al usuario que ingrese un nombre no vacio
    while (strlen(nuevo->nombre) == 0)
        leerCadena("  Nombre: ", nuevo->nombre, TAM_NOMBRE);
    nuevo->siguiente = listaClientes;
    listaClientes = nuevo;
    guardarClientes();
    printf(" Cliente registrado y guardado.\n");
    return nuevo;
}

// busca un cliente por su cedula y muestra su informacion si se encuentra, o un mensaje de error si no se encuentra
void buscarClienteMenu(void) {
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula a buscar: ", cedula, TAM_CEDULA);
    Cliente *cliente = buscarClientePorCedula(cedula);
    if (cliente == NULL) { printf(" Cliente no encontrado.\n"); return; }
    mostrarCliente(*cliente);
}
void listarClientes(void) {
    if (listaClientes == NULL) { printf(" No hay clientes registrados.\n"); return; }
    printf("\n--- LISTA DE CLIENTES ---\n");
    Cliente *cliente;
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
        mostrarCliente(*cliente);
}

// ELIMINACION de cliente con liberacion de memoria
void eliminarCliente(void) {
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula del cliente a eliminar: ", cedula, TAM_CEDULA);
    Cliente *actual = listaClientes, *anterior = NULL;
    while (actual != NULL && strcmp(actual->cedula, cedula) != 0) {
        anterior = actual;
        actual = actual->siguiente;
    }
    if (actual == NULL) { printf(" Cliente no encontrado.\n"); return; }
    if (anterior == NULL) listaClientes = actual->siguiente;
    else anterior->siguiente = actual->siguiente;
    free(actual);
    guardarClientes();
    printf("  >> Cliente eliminado.\n");
}

// funcion encargada de guardar ventas
void guardarVentas(void) {
    FILE *archivo = fopen(ARCH_VENTAS, "wb");
    if (archivo == NULL) { printf(" Error al abrir %s\n", ARCH_VENTAS); return; }
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)
        fwrite(venta, sizeof(Venta), 1, archivo);
    fclose(archivo);
}

void cargarVentas(void) {
    FILE *archivo = fopen(ARCH_VENTAS, "rb");
    if (archivo == NULL) return;
    Venta leida, *ultima = NULL;
    while (fread(&leida, sizeof(Venta), 1, archivo) == 1) {
        Venta *nueva = (Venta *)malloc(sizeof(Venta));
        if (nueva == NULL) break;
        *nueva = leida;
        nueva->siguiente = NULL;
        if (listaVentas == NULL) listaVentas = nueva;
        else ultima->siguiente = nueva;
        ultima = nueva;
        if (leida.id >= siguienteIdVenta) siguienteIdVenta = leida.id + 1;
    }
    fclose(archivo);
}

//funcion encargada en imprimir venta
void imprimirVenta(const Venta *venta) {
    printf("  #%-4d %s | %s (%s) | %s x%d | Sub: %.2f Desc: %.2f IVA: %.2f | TOTAL: %.2f\n",
           venta->id, venta->fecha, venta->nombreCliente, venta->cedulaCliente,
           venta->nombreProducto, venta->cantidad, venta->subtotal,
           venta->descuento, venta->valorIVA, venta->total);
}

// funcion para factura (en consola)
void imprimirFactura(const Venta *venta) {
    printf("\n  ==========================================\n");
    printf("             POLI POS - FACTURA\n");
    printf("  ==========================================\n");
    printf("  Factura No : %06d\n", venta->id);
    printf("  Fecha      : %s\n", venta->fecha);
    printf("  Cliente    : %s\n", venta->nombreCliente);
    printf("  Cedula     : %s\n", venta->cedulaCliente);
    printf("  ------------------------------------------\n");
    printf("  Producto   : %s (cod. %d)\n", venta->nombreProducto, venta->codigoProducto);
    printf("  Cantidad   : %d x %.2f USD\n", venta->cantidad, venta->precioUnitario);
    printf("  ------------------------------------------\n");
    printf("  SUBTOTAL   : %10.2f USD\n", venta->subtotal);
    printf("  DESCUENTO  : %10.2f USD\n", venta->descuento);
    printf("  IVA        : %10.2f USD\n", venta->valorIVA);
    printf("  TOTAL      : %10.2f USD\n", venta->total);
    printf("  ==========================================\n");
    printf("        Gracias por su compra\n");
    printf("  ==========================================\n");
}

//NUEVO: el programa genera la factura como ARCHIVO DE TEXTO automaticamente.
//Cada venta crea su propio archivo: factura_000001.txt, factura_000002.txt, ...
void guardarFacturaTxt(const Venta *venta) {
    char nombreArchivo[40];
    //sprintf arma el nombre del archivo usando el numero de la factura
    sprintf(nombreArchivo, "factura_%06d.txt", venta->id);
    //"w" crea el archivo de texto (si existiera, lo reemplaza)
    FILE *archivo = fopen(nombreArchivo, "w");
    if (archivo == NULL) { printf(" No se pudo crear %s\n", nombreArchivo); return; }
    //fprintf escribe en el archivo con el mismo formato que printf en consola
    fprintf(archivo, "==========================================\n");
    fprintf(archivo, "           POLI POS - FACTURA\n");
    fprintf(archivo, "==========================================\n");
    fprintf(archivo, "Factura No : %06d\n", venta->id);
    fprintf(archivo, "Fecha      : %s\n", venta->fecha);
    fprintf(archivo, "Cliente    : %s\n", venta->nombreCliente);
    fprintf(archivo, "Cedula     : %s\n", venta->cedulaCliente);
    fprintf(archivo, "------------------------------------------\n");
    fprintf(archivo, "Producto   : %s (cod. %d)\n", venta->nombreProducto, venta->codigoProducto);
    fprintf(archivo, "Cantidad   : %d x %.2f USD\n", venta->cantidad, venta->precioUnitario);
    fprintf(archivo, "------------------------------------------\n");
    fprintf(archivo, "SUBTOTAL   : %10.2f USD\n", venta->subtotal);
    fprintf(archivo, "DESCUENTO  : %10.2f USD\n", venta->descuento);
    fprintf(archivo, "IVA        : %10.2f USD\n", venta->valorIVA);
    fprintf(archivo, "TOTAL      : %10.2f USD\n", venta->total);
    fprintf(archivo, "==========================================\n");
    fprintf(archivo, "      Gracias por su compra\n");
    fprintf(archivo, "==========================================\n");
    fclose(archivo);
    printf(" Factura guardada como archivo de texto: %s\n", nombreArchivo);
}

//funcion registro de venta
void registrarVenta(void) {
    if (!caja.abierta) {     //logica de caja
        printf("\n La CAJA esta CERRADA. Debe abrir caja antes de vender.\n");
        return;
    }
    printf("\n--- REGISTRO DE VENTA ---\n");
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula del cliente: ", cedula, TAM_CEDULA);
    Cliente *cliente = buscarClientePorCedula(cedula);        /* [R10] secuencial */
    if (cliente == NULL) {
        printf("Cliente no registrado. Registrelo primero:\n");
        cliente = registrarCliente();
        if (cliente == NULL) return;
    }
    int codigo = leerEnteroPositivo("  Codigo del producto: ");
    Producto *producto = buscarProductoBinaria(codigo);
    if (producto == NULL) { printf(" Producto no encontrado.\n"); return; }
    /* 3. [R04][R09] CONTROL DE STOCK antes de procesar */
    printf("  Producto: %s | Precio: %.2f | Stock disponible: %d\n",
           producto->nombre, producto->precio, producto->stock);
    if (producto->stock <= 0) { printf(" SIN STOCK disponible. Venta cancelada.\n"); return; }
    int cantidad;
    while (1) {
        cantidad = leerEnteroPositivo("  Cantidad a vender: ");   /* [R19] > 0 */
        if (cantidad <= producto->stock) break;    /* no se vende mas de lo que hay */
        printf(" Stock insuficiente (disponible: %d).\n", producto->stock);
    }
    /* 4. MODIFICADO: la fecha de la venta la da el SISTEMA automaticamente */
    char fechaVenta[TAM_FECHA];
    fechaHoy(fechaVenta);
    printf("  Fecha de la venta (sistema): %s\n", fechaVenta);
    /* 5. [R09] DESCUENTO en porcentaje, validado entre 0 y 100 */
    float porcentajeDescuento;
    while (1) {
        porcentajeDescuento = leerFlotante("  Descuento % (0 a 100): ");
        if (porcentajeDescuento >= 0.0f && porcentajeDescuento <= 100.0f) break;
        printf("  >> Descuento invalido: debe estar entre 0 y 100.\n");
    }
    Venta *venta = (Venta *)malloc(sizeof(Venta));
    if (venta == NULL) { printf("  >> Sin memoria.\n"); return; }
    venta->id = siguienteIdVenta++;
    strcpy(venta->fecha, fechaVenta);
    strcpy(venta->cedulaCliente, cliente->cedula);
    strcpy(venta->nombreCliente, cliente->nombre);
    venta->codigoProducto = producto->codigo;
    strcpy(venta->nombreProducto, producto->nombre);
    venta->cantidad       = cantidad;
    venta->precioUnitario = producto->precio;
    venta->subtotal       = redondear2(producto->precio * cantidad);
    venta->descuento      = redondear2(venta->subtotal * porcentajeDescuento / 100.0f);
    float baseImponible   = venta->subtotal - venta->descuento;   //tras descuento
    venta->valorIVA       = redondear2(baseImponible * producto->iva / 100.0f);
    venta->total          = redondear2(baseImponible + venta->valorIVA);
    if (venta->total <= 0.0f) {
        printf(" Total invalido (%.2f). Venta cancelada.\n", venta->total);
        free(venta);
        return;
    }
    ajustarStock(&producto->stock, -cantidad);
    guardarProductos();
    venta->siguiente = listaVentas;   /* enlaza el nuevo nodo al inicio de la lista */
    listaVentas = venta;
    guardarVentas();
    imprimirFactura(venta);      //factura mostrada en consola
    guardarFacturaTxt(venta);    //NUEVO: factura generada como archivo de texto
    printf(" Venta registrada. Stock restante de '%s': %d\n",
           producto->nombre, producto->stock);
}

int contarVentas(void) {
    int cantidad = 0;
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente) cantidad++;
    return cantidad;
}
void buscarVentasPorCliente(void) {
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula del cliente: ", cedula, TAM_CEDULA);
    int encontradas = 0;
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)
        if (strcmp(venta->cedulaCliente, cedula) == 0) {
            imprimirVenta(venta);
            encontradas++;
        }
    if (!encontradas) printf(" No hay ventas para ese cliente.\n");
}

void listarVentas(void) {
    if (listaVentas == NULL) { printf(" No hay ventas registradas.\n"); return; }
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)
        imprimirVenta(venta);
}
void verMisCompras(void) {
    int encontradas = 0;
    printf("\n--- MIS COMPRAS (%s) ---\n", clienteActual->nombre);
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)   /* [R10] */
        if (strcmp(venta->cedulaCliente, clienteActual->cedula) == 0) {
            imprimirVenta(venta);
            encontradas++;
        }
    if (!encontradas) printf(" Aun no tiene compras registradas.\n");
}

//Reportes avanzados
float sumarVentasEnRangoRec(Venta *nodo, const char *desde, const char *hasta) {
    if (nodo == NULL) return 0.0f;
    float aporte = 0.0f;
    if (strcmp(nodo->fecha, desde) >= 0 && strcmp(nodo->fecha, hasta) <= 0)
        aporte = nodo->total;
    return aporte + sumarVentasEnRangoRec(nodo->siguiente, desde, hasta);
}
FILE *abrirDestinoReporte(const char *nombreArchivo, int *esArchivo) {
    printf("  Salida del reporte: 1) Consola  2) Exportar a %s\n", nombreArchivo);
    int opcion = leerEnteroEnRango("  Opcion: ", 1, 2);
    if (opcion == 2) {
        FILE *archivo = fopen(nombreArchivo, "w");
        if (archivo != NULL) { *esArchivo = 1; return archivo; }
        printf(" No se pudo crear el archivo; se usara la consola.\n");
    }
    *esArchivo = 0;
    return stdout;
}

void reporteVentasPorRango(void) {
    char desde[TAM_FECHA], hasta[TAM_FECHA];
    leerFecha("  Fecha desde (AAAA-MM-DD): ", desde);
    while (1) {
        leerFecha("  Fecha hasta (AAAA-MM-DD): ", hasta);
        if (strcmp(hasta, desde) >= 0) break;
        printf(" La fecha final es menor a la inicial.\n");
    }
    int esArchivo = 0;
    FILE *salida = abrirDestinoReporte("reporte_ventas_rango.txt", &esArchivo);
    fprintf(salida, "===== REPORTE DE VENTAS DEL %s AL %s =====\n", desde, hasta);
    int encontradas = 0;
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente) {
        if (strcmp(venta->fecha, desde) >= 0 && strcmp(venta->fecha, hasta) <= 0) {
            fprintf(salida, "#%-4d %s | %s | %s x%d | TOTAL: %.2f\n",
                    venta->id, venta->fecha, venta->nombreCliente,
                    venta->nombreProducto, venta->cantidad, venta->total);
            encontradas++;
        }
    }
    float totalAcumulado = sumarVentasEnRangoRec(listaVentas, desde, hasta);
    fprintf(salida, "Ventas encontradas: %d | TOTAL ACUMULADO (recursivo): %.2f USD\n",
            encontradas, redondear2(totalAcumulado));
    if (esArchivo) {
        fclose(salida);
        printf(" Reporte exportado a reporte_ventas_rango.txt\n");
    }
}
void burbujaMasVendidos(ItemVendido arreglo[], int total) {
    for (int pasada = 0; pasada < total - 1; pasada++)
        for (int indice = 0; indice < total - 1 - pasada; indice++)
            if (arreglo[indice].unidades < arreglo[indice + 1].unidades) {
                ItemVendido temporal = arreglo[indice];
                arreglo[indice] = arreglo[indice + 1];
                arreglo[indice + 1] = temporal;
            }
}
void reporteProductosMasVendidos(void) {
    int totalProductos = contarProductos();
    if (totalProductos == 0) { printf(" No hay productos.\n"); return; }
    ItemVendido *items = (ItemVendido *)calloc(totalProductos, sizeof(ItemVendido));
    if (items == NULL) return;
    int usados = 0;
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente) {
        int posicion = -1;
        for (int indice = 0; indice < usados; indice++)
            if (items[indice].codigo == venta->codigoProducto) { posicion = indice; break; }
        if (posicion == -1) {
            if (usados >= totalProductos) continue;
            posicion = usados++;
            items[posicion].codigo = venta->codigoProducto;
            strcpy(items[posicion].nombre, venta->nombreProducto);
            items[posicion].unidades = 0;
            items[posicion].monto = 0.0f;
        }
        items[posicion].unidades += venta->cantidad;
        items[posicion].monto    += venta->total;
    }
    if (usados == 0) { printf(" Aun no hay ventas.\n"); free(items); return; }
    burbujaMasVendidos(items, usados);
    int esArchivo = 0;
    FILE *salida = abrirDestinoReporte("reporte_mas_vendidos.txt", &esArchivo);
    fprintf(salida, "===== PRODUCTOS MAS VENDIDOS (burbuja descendente) =====\n");
    fprintf(salida, "%-4s %-8s %-25s %10s %12s\n", "Pos", "Codigo", "Producto", "Unidades", "Monto USD");
    for (int indice = 0; indice < usados; indice++)
        fprintf(salida, "%-4d %-8d %-25s %10d %12.2f\n",
                indice + 1, items[indice].codigo, items[indice].nombre,
                items[indice].unidades, redondear2(items[indice].monto));
    if (esArchivo) { fclose(salida); printf(" Reporte exportado a reporte_mas_vendidos.txt\n"); }
    free(items);
}

// burbuja ascendente
void burbujaPorCantidad(Producto *arreglo[], int total) {
    for (int pasada = 0; pasada < total - 1; pasada++)
        for (int indice = 0; indice < total - 1 - pasada; indice++)
            if (arreglo[indice]->stock > arreglo[indice + 1]->stock) {
                Producto *temporal = arreglo[indice];
                arreglo[indice] = arreglo[indice + 1];
                arreglo[indice + 1] = temporal;
            }
}

void reporteStockBajo(void) {
    int totalProductos = contarProductos(), bajos = 0;
    if (totalProductos == 0) { printf(" No hay productos.\n"); return; }
    Producto **arreglo = (Producto **)malloc(totalProductos * sizeof(Producto *));
    if (arreglo == NULL) return;
    Producto *producto;
    for (producto = listaProductos; producto != NULL; producto = producto->siguiente)
        if (producto->stock < producto->stockMinimo)
            arreglo[bajos++] = producto;
    if (bajos == 0) {
        printf(" Excelente: ningun producto esta bajo el stock minimo.\n");
        free(arreglo);
        return;
    }
    burbujaPorCantidad(arreglo, bajos);
    int esArchivo = 0;
    FILE *salida = abrirDestinoReporte("reporte_stock_bajo.txt", &esArchivo);
    fprintf(salida, "===== STOCK BAJO MINIMO (ordenado por cantidad) =====\n");
    fprintf(salida, "%-8s %-25s %8s %8s %10s\n", "Codigo", "Producto", "Stock", "Minimo", "Faltante");
    for (int indice = 0; indice < bajos; indice++)
        fprintf(salida, "%-8d %-25s %8d %8d %10d\n",
                arreglo[indice]->codigo, arreglo[indice]->nombre,
                arreglo[indice]->stock, arreglo[indice]->stockMinimo,
                arreglo[indice]->stockMinimo - arreglo[indice]->stock);
    if (esArchivo) { fclose(salida); printf(" Reporte exportado a reporte_stock_bajo.txt\n"); }
    free(arreglo);
}

void seleccionRanking(ClienteRank arreglo[], int total) {
    for (int indice = 0; indice < total - 1; indice++) {
        int posicionMayor = indice;
        for (int comparado = indice + 1; comparado < total; comparado++)
            if (arreglo[comparado].monto > arreglo[posicionMayor].monto)
                posicionMayor = comparado;
        if (posicionMayor != indice) {
            ClienteRank temporal = arreglo[indice];
            arreglo[indice] = arreglo[posicionMayor];
            arreglo[posicionMayor] = temporal;
        }
    }
}
void reporteRankingClientes(void) {
    int totalVentas = contarVentas();
    if (totalVentas == 0) { printf(" No hay ventas registradas.\n"); return; }
    ClienteRank *ranking = (ClienteRank *)calloc(totalVentas, sizeof(ClienteRank));
    if (ranking == NULL) return;
    int usados = 0;
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente) {
        int posicion = -1;
        for (int indice = 0; indice < usados; indice++)
            if (strcmp(ranking[indice].cedula, venta->cedulaCliente) == 0) {
                posicion = indice;
                break;
            }
        if (posicion == -1) {
            posicion = usados++;
            strcpy(ranking[posicion].cedula, venta->cedulaCliente);
            strcpy(ranking[posicion].nombre, venta->nombreCliente);
            ranking[posicion].monto = 0.0f;
            ranking[posicion].compras = 0;
        }
        ranking[posicion].monto += venta->total;
        ranking[posicion].compras++;
    }
    seleccionRanking(ranking, usados);
    int esArchivo = 0;
    FILE *salida = abrirDestinoReporte("reporte_ranking_clientes.txt", &esArchivo);
    fprintf(salida, "===== RANKING DE CLIENTES POR MONTO DE COMPRAS =====\n");
    fprintf(salida, "%-4s %-12s %-25s %8s %12s\n", "Pos", "Cedula", "Cliente", "Compras", "Monto USD");
    for (int indice = 0; indice < usados; indice++)
        fprintf(salida, "%-4d %-12s %-25s %8d %12.2f\n",
                indice + 1, ranking[indice].cedula, ranking[indice].nombre,
                ranking[indice].compras, redondear2(ranking[indice].monto));
    if (esArchivo) { fclose(salida); printf(" Reporte exportado a reporte_ranking_clientes.txt\n"); }
    free(ranking);
}
//funcion caja
void guardarCaja(void) {
    FILE *archivo = fopen(ARCH_CAJA, "wb");
    if (archivo == NULL) return;
    fwrite(&caja, sizeof(EstadoCaja), 1, archivo);
    fclose(archivo);
}

void cargarCaja(void) {
    FILE *archivo = fopen(ARCH_CAJA, "rb");
    if (archivo == NULL) return;
    if (fread(&caja, sizeof(EstadoCaja), 1, archivo) != 1)
        printf(" Aviso: no se pudo leer el estado previo de la caja.\n");
    fclose(archivo);
}
void abrirCaja(void) {
    if (caja.abierta) {
        printf(" La caja YA esta abierta con %.2f USD.\n", caja.montoApertura);
        return;
    }
    printf("\n--- APERTURA DE CAJA ---\n");
    caja.montoApertura = redondear2(leerDineroPositivo("  Monto de apertura : "));
    caja.idVentaInicio = siguienteIdVenta;
    caja.abierta = 1;
    guardarCaja();
    printf(" Caja ABIERTA con %.2f USD.\n", caja.montoApertura);
}

//recursividad
float totalVentasDesdeIdRec(Venta *nodo, int idInicio) {
    if (nodo == NULL) return 0.0f;
    float aporte = (nodo->id >= idInicio) ? nodo->total : 0.0f;
    return aporte + totalVentasDesdeIdRec(nodo->siguiente, idInicio);
}

// RECURSIVO: cuenta las ventas desde un ID de venta en adelante
int contarVentasDesdeIdRec(Venta *nodo, int idInicio) {
    if (nodo == NULL) return 0;
    return ((nodo->id >= idInicio) ? 1 : 0)
           + contarVentasDesdeIdRec(nodo->siguiente, idInicio);
}

void cerrarCaja(void) {
    if (!caja.abierta) { printf(" La caja no esta abierta.\n"); return; }
    printf("\n--- CIERRE DE CAJA ---\n");
    int   ventasDelTurno   = contarVentasDesdeIdRec(listaVentas, caja.idVentaInicio);
    float totalDelTurno    = redondear2(totalVentasDesdeIdRec(listaVentas, caja.idVentaInicio));
    float efectivoEsperado = redondear2(caja.montoApertura + totalDelTurno);
    printf("  ========= CONSOLIDADO DE CIERRE =========\n");
    printf("  Monto de apertura        : %.2f USD\n", caja.montoApertura);
    printf("  Ventas del turno         : %d\n", ventasDelTurno);
    printf("  Total vendido            : %.2f USD\n", totalDelTurno);
    printf("  Efectivo esperado en caja: %.2f USD\n", efectivoEsperado);
    printf("  =========================================\n");
    FILE *archivo = fopen(ARCH_CIERRES, "a");    /* [R02] "a": anade al final */
    if (archivo != NULL) {
        fprintf(archivo, "CIERRE | Apertura: %.2f | Ventas: %d | Total: %.2f | Esperado: %.2f\n",
                caja.montoApertura, ventasDelTurno, totalDelTurno, efectivoEsperado);
        fclose(archivo);
        printf(" Consolidado guardado en %s\n", ARCH_CIERRES);
    }
    caja.abierta = 0;
    caja.montoApertura = 0.0f;
    caja.idVentaInicio = 0;
    guardarCaja();
    printf(" Caja CERRADA correctamente.\n");
}

//roles
int menuPortalCliente(void) {
    while (1) {
        printf("\n============ PORTAL DEL CLIENTE ============\n");
        printf("  Sesion: %s (%s)\n", clienteActual->nombre, clienteActual->cedula);
        printf("--------------------------------------------\n");
        printf(" 1) Ver catalogo de productos y precios\n");
        printf(" 2) Mis compras \n");
        printf(" 7) Cambiar de usuario\n");
        printf(" 0) Salir\n");
        int opcion = leerEnteroEnRango("Opcion: ", 0, 7);
        if (opcion == 1) listarProductos();
        else if (opcion == 2) verMisCompras();
        else if (opcion == 0 || opcion == 7) return opcion;
        else printf("  >> Opcion invalida.\n");
    }
}
void menuProductos(int rol) {
    if (rol == ROL_CAJERO) {
        while (1) {
            printf("\n===== PRODUCTOS (CAJERO) =====\n");
            printf(" 1) Listar productos\n");
            printf(" 0) Volver\n");
            int opcion = leerEnteroEnRango("Opcion: ", 0, 1);
            if (opcion == 0) return;
            listarProductos();
        }
    }
    // administrador
    while (1) {
        printf("\n===== PRODUCTOS (%s) =====\n", nombreRol(rol));
        printf(" 1) Registrar producto\n");
        printf(" 2) Listar productos\n");
        printf(" 3) Editar producto\n");
        printf(" 4) Eliminar producto\n");
        printf(" 5) Buscar por codigo \n");
        printf(" 0) Volver\n");
        int opcion = leerEnteroEnRango("Opcion: ", 0, 5);
        if (opcion == 0) return;
        else if (opcion == 1) registrarProducto();
        else if (opcion == 2) listarProductos();
        else if (opcion == 3) editarProducto();
        else if (opcion == 4) eliminarProducto();
        else if (opcion == 5) {
            int codigo = leerEnteroPositivo("  Codigo: ");
            Producto *encontrado = buscarProductoBinaria(codigo);
            if (encontrado == NULL) printf(" No encontrado.\n");
            else printf("  Encontrado: %s | Precio %.2f | Stock %d | IVA %d%%\n",
                        encontrado->nombre, encontrado->precio,
                        encontrado->stock, encontrado->iva);
        }
    }
}

void menuInventario(int rol) {
    while (1) {
        printf("\n===== INVENTARIO (%s) =====\n", nombreRol(rol));
        printf(" 1) Registrar compra \n");
        printf(" 2) Ajuste de stock\n");
        printf(" 3) Consultar historico de movimientos\n");
        printf(" 0) Volver\n");
        int opcion = leerEnteroEnRango("Opcion: ", 0, 3);
        if (opcion == 0) return;
        else if (opcion == 1) registrarCompra();
        else if (opcion == 2) registrarAjuste();
        else if (opcion == 3) consultarHistoricoInventario();
    }
}
void menuClientes(int rol) {
    while (1) {
        printf("\n===== CLIENTES (%s) =====\n", nombreRol(rol));
        printf(" 1) Registrar cliente\n");
        printf(" 2) Buscar cliente por cedula\n");
        if (rol == ROL_ADMIN) {
            printf(" 3) Listar clientes\n");
            printf(" 4) Eliminar cliente\n");
        }
        printf(" 0) Volver\n");
        int maximo = (rol == ROL_ADMIN) ? 4 : 2;
        int opcion = leerEnteroEnRango("Opcion: ", 0, maximo);
        if (opcion == 0) return;
        else if (opcion == 1) registrarCliente();
        else if (opcion == 2) buscarClienteMenu();
        else if (opcion == 3) listarClientes();
        else if (opcion == 4) eliminarCliente();
    }
}
void menuVentas(int rol) {
    while (1) {
        printf("\n===== VENTAS (%s) =====\n", nombreRol(rol));
        printf(" 1) Registrar venta\n");
        if (rol == ROL_ADMIN) {
            printf(" 2) Listar ventas\n");
            printf(" 3) Buscar ventas por cliente\n");
        }
        printf(" 0) Volver\n");
        int maximo = (rol == ROL_ADMIN) ? 3 : 1;
        int opcion = leerEnteroEnRango("Opcion: ", 0, maximo);
        if (opcion == 0) return;
        else if (opcion == 1) registrarVenta();
        else if (opcion == 2) listarVentas();
        else if (opcion == 3) buscarVentasPorCliente();
    }
}

void menuReportes(int rol) {
    if (rol == ROL_INVENTARIO) {
        printf("\n===== REPORTES (ENCARGADO: solo stock bajo minimo) =====\n");
        reporteStockBajo();
        return;
    }
    while (1) {
        printf("\n===== REPORTES AVANZADOS (ADMINISTRADOR) =====\n");
        printf(" 1) Ventas por rango de fechas\n");
        printf(" 2) Productos mas vendidos\n");
        printf(" 3) Stock bajo minimo\n");
        printf(" 4) Ranking de clientes por monto\n");
        printf(" 0) Volver\n");
        int opcion = leerEnteroEnRango("Opcion: ", 0, 4);
        if (opcion == 0) return;
        else if (opcion == 1) reporteVentasPorRango();
        else if (opcion == 2) reporteProductosMasVendidos();
        else if (opcion == 3) reporteStockBajo();
        else if (opcion == 4) reporteRankingClientes();
    }
}

void menuCaja(int rol) {
    while (1) {
        printf("\n===== CAJA (%s) ===== [estado: %s]\n",
               nombreRol(rol), caja.abierta ? "ABIERTA" : "CERRADA");
        printf(" 1) Abrir caja\n");
        printf(" 2) Cerrar caja (consolidado de ventas)\n");
        printf(" 0) Volver\n");
        int opcion = leerEnteroEnRango("Opcion: ", 0, 2);
        if (opcion == 0) return;
        else if (opcion == 1) abrirCaja();
        else if (opcion == 2) cerrarCaja();
    }
}

int iniciarSesion(void) {
    printf("=========================================================\n");
    printf("   Punto de venta \"Estilo en Fuga\"  \n");
    printf("=========================================================\n");
    printf(" 1) CLIENTE (ingreso con su cedula)\n");
    printf(" 2) PERSONAL (ingreso con contrasena)\n");
    int tipoUsuario = leerEnteroEnRango("Seleccione una opcion: ", 1, 2);

    if (tipoUsuario == 1) {
        char cedula[TAM_CEDULA];
        while (1) {
            leerCadena("Ingrese su cedula: ", cedula, TAM_CEDULA);
            if (!validarCedulaEcuatoriana(cedula)) {
                printf(">> Cedula ecuatoriana invalida.\n");
                continue;
            }
            clienteActual = buscarClientePorCedula(cedula);
            if (clienteActual == NULL) {
                printf("Cedula no registrada. Registrese a continuacion:\n");
                clienteActual = registrarCliente();
                if (clienteActual == NULL) continue;
            }
            break;
        }
        printf("\n>> Bienvenido %s, entraste como: CLIENTE\n", clienteActual->nombre);
        return ROL_CLIENTE;
    }
    char clave[40];
    int intentosRestantes = 3;
    while (intentosRestantes > 0) {
        leerCadena("Ingrese su contrasena: ", clave, sizeof(clave));
        if (strcmp(clave, CLAVE_ADMIN) == 0) {
            printf("\n Bienvenido, entraste como: ADMINISTRADOR\n");
            return ROL_ADMIN;
        } else if (strcmp(clave, CLAVE_CAJERO) == 0) {
            printf("\n>> Bienvenido, entraste como: CAJERO\n");
            return ROL_CAJERO;
        } else if (strcmp(clave, CLAVE_INVENTARIO) == 0) {
            printf("\n>> Bienvenido, entraste como: ENCARGADO DE INVENTARIO\n");
            return ROL_INVENTARIO;
        }
        intentosRestantes--;
        printf(" Contrasena incorrecta. Intentos restantes: %d\n", intentosRestantes);
    }
    printf(" Acceso bloqueado por seguridad. El programa terminara.\n");
    exit(EXIT_FAILURE);
}

void liberarListas(void) {
    while (listaProductos != NULL) {
        Producto *nodoALiberar = listaProductos;
        listaProductos = listaProductos->siguiente;
        free(nodoALiberar);
    }
    while (listaClientes != NULL) {
        Cliente *nodoALiberar = listaClientes;
        listaClientes = listaClientes->siguiente;
        free(nodoALiberar);
    }
    while (listaVentas != NULL) {
        Venta *nodoALiberar = listaVentas;
        listaVentas = listaVentas->siguiente;
        free(nodoALiberar);
    }
}


int main(void) {
    cargarProductos();
    cargarClientes();
    cargarVentas();
    cargarCaja();

    int rol = iniciarSesion();

    while (1) {
        if (rol == ROL_CLIENTE) {
            int opcionPortal = menuPortalCliente();
            if (opcionPortal == 7) {
                clienteActual = NULL;
                rol = iniciarSesion();
                continue;
            }
            liberarListas();
            printf("\n Gracias por usar POLI POS. Datos guardados.\n");
            return 0;
        }

        printf("\n=================== MENU PRINCIPAL ===================\n");
        printf("  Sesion activa: %s | Caja: %s\n", nombreRol(rol),
               caja.abierta ? "ABIERTA" : "CERRADA");
        printf("------------------------------------------------------\n");
        if (rolTieneAcceso(rol, 1))
            printf(" 1) Productos%s\n", rol == ROL_CAJERO ? " (solo listar)" : "");
        if (rolTieneAcceso(rol, 2)) printf(" 2) Inventario\n");
        if (rolTieneAcceso(rol, 3)) printf(" 3) Clientes\n");
        if (rolTieneAcceso(rol, 4)) printf(" 4) Ventas\n");
        if (rolTieneAcceso(rol, 5))
            printf(" 5) Reportes%s\n",
                   rol == ROL_INVENTARIO ? " (solo stock bajo minimo)" : " avanzados");
        if (rolTieneAcceso(rol, 6)) printf(" 6) Caja\n");
        printf(" 7) Cambiar de usuario\n");
        printf(" 0) Salir\n");
        int opcion = leerEnteroEnRango("Opcion: ", 0, 7);

        if (opcion >= 1 && opcion <= 6 && !rolTieneAcceso(rol, opcion)) {
            accesoDenegado();
            continue;
        }

        if (opcion == 1) menuProductos(rol);
        else if (opcion == 2) menuInventario(rol);
        else if (opcion == 3) menuClientes(rol);
        else if (opcion == 4) menuVentas(rol);
        else if (opcion == 5) menuReportes(rol);
        else if (opcion == 6) menuCaja(rol);
        else if (opcion == 7) rol = iniciarSesion();
        else {
            liberarListas();
            printf("\n Gracias por usar POLI POS. Datos guardados.\n");
            return 0;
        }
    }
}