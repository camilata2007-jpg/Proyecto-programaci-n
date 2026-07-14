#include "poli_pos.h"
void limpiarBuffer(void) {
    int caracter;
    while ((caracter = getchar()) != '\n' && caracter != EOF);
}


void leerCadena(const char *mensaje, char *destino, int tamanio) {
    printf("%s", mensaje);
    if (fgets(destino, tamanio, stdin) == NULL) {   
        printf("\n Entrada finalizada. Cerrando el sistema de forma segura.\n");
        exit(EXIT_SUCCESS);
    }
    size_t longitud = strlen(destino);
    if (longitud > 0 && destino[longitud - 1] == '\n')
        destino[longitud - 1] = '\0';              
    else
        limpiarBuffer();                           
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


int leerEnteroPositivo(const char *mensaje) {
    int valor;
    while (1) {
        valor = leerEntero(mensaje);
        if (valor > 0) return valor;
        printf(" El valor debe ser MAYOR a cero .\n");
    }
}


int leerEnteroEnRango(const char *mensaje, int minimo, int maximo) {
    int valor;
    while (1) {
        valor = leerEntero(mensaje);
        if (valor >= minimo && valor <= maximo) return valor;
        printf(" Opcion fuera de rango (%d - %d).\n", minimo, maximo);
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
        printf(" Monto invalido: el dinero debe ser MAYOR a cero.\n");
    }
}


float redondear2(float valor) {
    return (float)((long)(valor * 100.0f + 0.5f)) / 100.0f;
}


int esFechaValida(const char *fecha) {
    int anio, mes, dia;
    if (strlen(fecha) != 10 || fecha[4] != '-' || fecha[7] != '-') return 0;
    if (sscanf(fecha, "%4d-%2d-%2d", &anio, &mes, &dia) != 3) return 0;
    if (anio < 2000 || anio > 2100) return 0;
    if (mes < 1 || mes > 12) return 0;
    if (dia < 1 || dia > 31) return 0;
    return 1;
}


void leerFecha(const char *mensaje, char *destino) {
    while (1) {
        leerCadena(mensaje, destino, TAM_FECHA);
        if (esFechaValida(destino)) return;
        printf(" Fecha invalida. Use el formato AAAA-MM-DD.\n");
    }
}

int validarCedulaEcuatoriana(const char *cedula) {
    int posicion, suma = 0;
    int provincia, tercerDigito, digitoVerificador, decenaSuperior;
    if (strlen(cedula) != 10) return 0;
    for (posicion = 0; posicion < 10; posicion++)          /* solo digitos */
        if (cedula[posicion] < '0' || cedula[posicion] > '9') return 0;
    provincia = (cedula[0] - '0') * 10 + (cedula[1] - '0');
    if (!((provincia >= 1 && provincia <= 24) || provincia == 30)) return 0;
    tercerDigito = cedula[2] - '0';
    if (tercerDigito >= 6) return 0;
    for (posicion = 0; posicion < 9; posicion++) {
        int digito = cedula[posicion] - '0';
        int producto = (posicion % 2 == 0) ? digito * 2 : digito * 1;
        if (producto > 9) producto -= 9;                  
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
        printf(" IVA invalido: los unicos valores permitidos son 0, 12 y 15.\n");
    }
}

const char *nombreRol(int rol) {
    if (rol == ROL_ADMIN)   return "ADMINISTRADOR";
    if (rol == ROL_CAJERO)  return "CAJERO";
    if (rol == ROL_CLIENTE) return "CLIENTE";
    return "ENCARGADO DE INVENTARIO";
}


void accesoDenegado(void) {
    printf("\n  [ACCESO DENEGADO] Su rol no tiene permisos para esta operacion.\n");
}

int rolTieneAcceso(int rol, int modulo) {
    if (rol == ROL_ADMIN) return 1;                    
    if (rol == ROL_CAJERO)                             
        return modulo == 1 || modulo == 3 || modulo == 4 || modulo == 6;
    return modulo == 1 || modulo == 2 || modulo == 5;
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