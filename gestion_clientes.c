#include "poli_pos.h"
 
//FUNCION PARA GUARDAR LA LISTA DE CLIENTES EN EL ARCHIVO BINARIO:
void guardarClientes(void) 
{
    FILE *archivo = fopen(ARCH_CLIENTES, "wb");
    if (archivo == NULL) 
    { 
        printf("  Error al abrir %s\n", ARCH_CLIENTES); 

    return; 
    }
    Cliente *cliente;
    //BLUCLE PARA ESCRIBIR CADA CLIENTE DE LA LISTA EN EL ARCHIVO:
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
        fwrite(cliente, sizeof(Cliente), 1, archivo);
    fclose(archivo);
}
 
//FUNCION PARA CARGAR LA LISTA DE CLIENTES DESDE EL ARCHIVO BINARIO:
void cargarClientes(void) 
{
    FILE *archivo = fopen(ARCH_CLIENTES, "rb");
    if (archivo == NULL) 

return;
    Cliente leido, *ultimo = NULL;
    while (fread(&leido, sizeof(Cliente), 1, archivo) == 1) 
    {
        Cliente *nuevo = (Cliente *)malloc(sizeof(Cliente)); 
        if (nuevo == NULL) 
        break;
        *nuevo = leido;
        nuevo->siguiente = NULL;
        if (listaClientes == NULL) listaClientes = nuevo;
        else ultimo->siguiente = nuevo;
        ultimo = nuevo;
    }
    fclose(archivo);
}
 
//FUNCION PARA MOSTRAR UN CLIENTE POR PANTALLA:
void mostrarCliente(Cliente cliente) 
{
    printf("  Cedula: %s | Nombre: %s\n", cliente.cedula, cliente.nombre);
}
 
//FUNCION PARA VERIFICAR SI UNA CEDULA YA ESTA REGISTRADA:
int cedulaYaRegistrada(char cedula[TAM_CEDULA]) 
{
    Cliente *cliente;
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
        if (strcmp(cliente->cedula, cedula) == 0)
        return 1; 
    return 0;
}
 
//FUNCION PARA BUSCAR UN CLIENTE POR CEDULA:
Cliente *buscarClientePorCedula(const char *cedula) 
{
    Cliente *cliente;
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
        if (strcmp(cliente->cedula, cedula) == 0) return cliente;
    return NULL;
}
 
//FUNCION PARA REGISTRAR UN NUEVO CLIENTE:
Cliente *registrarCliente(void) 
{
    char cedula[TAM_CEDULA];
    printf("\n--- REGISTRO DE CLIENTE ---\n");
    while (1) {
        leerCadena("  Cedula ecuatorina : ", cedula, TAM_CEDULA);
        if (!validarCedulaEcuatoriana(cedula)) 
        {            
            printf("  >> Cedula ecuatoriana INVALIDA.\n");
            continue;
        }
        if (cedulaYaRegistrada(cedula)) 
        {                   
            printf("  >> Esa cedula ya esta registrada.\n");
            continue;
        }
        break;
    }
    Cliente *nuevo = (Cliente *)malloc(sizeof(Cliente));     
    if (nuevo == NULL) 
    { 
        printf("  >> Sin memoria.\n"); 

    return NULL; 
    }
    strcpy(nuevo->cedula, cedula);
    leerCadena("  Nombre completo: ", nuevo->nombre, TAM_NOMBRE);
    while (strlen(nuevo->nombre) == 0)
        leerCadena("  Nombre (no vacio): ", nuevo->nombre, TAM_NOMBRE);
    nuevo->siguiente = listaClientes;                     
    listaClientes = nuevo;
    guardarClientes();                                  
    printf(" Cliente registrado y guardado.\n");
    return nuevo;
}
 
//FUNCION PARA BUSCAR UN CLIENTE POR CEDULA Y MOSTRARLO:
void buscarClienteMenu(void) 
{
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula a buscar: ", cedula, TAM_CEDULA);
    Cliente *cliente = buscarClientePorCedula(cedula);
    if (cliente == NULL) 
    { 
        printf(" Cliente no encontrado.\n"); 

    return; 
 }
    mostrarCliente(*cliente);  
}
 
//FUNCION PARA LISTAR TODOS LOS CLIENTES:
void listarClientes(void) 
{
    if (listaClientes == NULL) 
    { 
        printf("  >> No hay clientes registrados.\n"); 

    return; 
    }
    printf("\n--- LISTA DE CLIENTES ---\n");
    Cliente *cliente;
    for (cliente = listaClientes; cliente != NULL; cliente = cliente->siguiente)
        mostrarCliente(*cliente);                    
}
 
//FUNCION PARA ELIMINAR UN CLIENTE DE LA LISTA Y DEL ARCHIVO:
void eliminarCliente(void) 
{
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula del cliente a eliminar: ", cedula, TAM_CEDULA);
    Cliente *actual = listaClientes, *anterior = NULL;
    while (actual != NULL && strcmp(actual->cedula, cedula) != 0) 
    {
        anterior = actual;
        actual = actual->siguiente;
    }
    if (actual == NULL) 
    { 
        printf(" Cliente no encontrado.\n"); 
        
    return; 
    }
    if (anterior == NULL) listaClientes = actual->siguiente;
    else anterior->siguiente = actual->siguiente;        
    free(actual);
    guardarClientes();
    printf(" Cliente eliminado.\n");
}