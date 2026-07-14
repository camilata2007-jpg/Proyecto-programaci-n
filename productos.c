#include "poli_pos.h"
 
//FUNCION PARA GUARDAR LA LISTA DE PRODUCTOS EN EL ARCHIVO BINARIO:
void guardarProductos(void) 
{
    FILE *archivo = fopen(ARCH_PRODUCTOS, "wb");  
    if (archivo == NULL) 
    { 
        printf(" Error al abrir %s\n", ARCH_PRODUCTOS); return; 
    }
    Producto *producto = listaProductos;
    while (producto != NULL) 
    {                     
        fwrite(producto, sizeof(Producto), 1, archivo);
        producto = producto->siguiente;
    }
    fclose(archivo);
}
 
//FUNCION PARA CARGAR LA LISTA DE PRODUCTOS DESDE EL ARCHIVO BINARIO:
void cargarProductos(void) 
{
    FILE *archivo = fopen(ARCH_PRODUCTOS, "rb");  
    if (archivo == NULL) 
    { 
        printf(" Error al abrir %s\n", ARCH_PRODUCTOS); return; 
    }
    Producto leido, *ultimo = NULL;
    while (fread(&leido, sizeof(Producto), 1, archivo) == 1) 
    {  
        Producto *nuevo = (Producto *)malloc(sizeof(Producto)); 
        if (nuevo == NULL) 
        { 
            printf(" Sin memoria.\n"); break; 
        }
        *nuevo = leido;    
        nuevo->siguiente = NULL;
        if (listaProductos == NULL) listaProductos = nuevo;  
        else ultimo->siguiente = nuevo;       
        ultimo = nuevo;
    }
    fclose(archivo);
}
 
//FUNCION PARA BUSCAR UN PRODUCTO POR CODIGO:
Producto *buscarProductoSecuencial(int codigo) 
{
    Producto *producto = listaProductos;
    while (producto != NULL) 
    {
        if (producto->codigo == codigo) return producto;  
        producto = producto->siguiente;                   
    }
    return NULL;                                       
}
 
//FUNCION PARA CONTAR LA CANTIDAD DE PRODUCTOS EN LA LISTA:
int contarProductos(void) 
{
    int cantidad = 0;
    Producto *producto;
    for (producto = listaProductos; producto != NULL; producto = producto->siguiente)
        cantidad++;
    return cantidad;
}
 
//FUNCION PARA BUSCAR UN PRODUCTO POR CODIGO:
Producto *buscarProductoBinaria(int codigoBuscado) 
{
int totalProductos = contarProductos();
int posicion = 0;
if (totalProductos == 0) return NULL;
Producto **arreglo = (Producto **)malloc(totalProductos * sizeof(Producto *));

if (arreglo == NULL) return NULL;              
Producto *producto;

//BUCLE PARA LLENAR EL ARREGLO CON PUNTEROS A LOS PRODUCTOS DE LA LISTA:
for (producto = listaProductos; producto != NULL; producto = producto->siguiente)
    arreglo[posicion++] = producto; 

    for (int pasada = 0; pasada < totalProductos - 1; pasada++)
        for (int indice = 0; indice < totalProductos - 1 - pasada; indice++)
            if (arreglo[indice]->codigo > arreglo[indice + 1]->codigo) 
            {
                Producto *temporal = arreglo[indice];
                arreglo[indice] = arreglo[indice + 1];
                arreglo[indice + 1] = temporal;
            }


int izquierda = 0, derecha = totalProductos - 1;
Producto *resultado = NULL;
while (izquierda <= derecha) {
    int centro = (izquierda + derecha) / 2;
    if (arreglo[centro]->codigo == codigoBuscado) 
    {   
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
 
//REGISTRO DE PRODUCTO, EDICION Y ELIMINACION:
void registrarProducto(void) 
{
    Producto *nuevo = (Producto *)malloc(sizeof(Producto));

    if (nuevo == NULL) 
    { 
        printf(" Sin memoria.\n"); return; 
    }
    printf("\n--- REGISTRO DE PRODUCTO ---\n");
    while (1) 
    {
        nuevo->codigo = leerEnteroPositivo("  Codigo del producto: ");
        if (buscarProductoSecuencial(nuevo->codigo) == NULL) 
        break;
        printf(" Ya existe un producto con ese codigo.\n");
    }

    leerCadena("  Nombre: ", nuevo->nombre, TAM_NOMBRE);
    while (strlen(nuevo->nombre) == 0)
    {
        leerCadena("  Nombre: ", nuevo->nombre, TAM_NOMBRE);
    }
    nuevo->precio = redondear2(leerDineroPositivo("  Precio unitario (USD): ")); 
    nuevo->stock = leerEnteroPositivo("  Stock inicial: ");                     
    nuevo->stockMinimo = leerEnteroPositivo("  Stock minimo (alerta): ");             
    nuevo->iva = leerIVA();                                                   
    nuevo->siguiente = listaProductos;  
    listaProductos = nuevo;
    guardarProductos();                                                          
    printf(" Producto registrado y guardado en archivo binario.\n");
}
 
//FUNCION PARA EDITAR UN PRODUCTO EXISTENTE:
void editarProducto(void) 
{
int codigo = leerEnteroPositivo("  Codigo del producto a editar: ");
Producto *producto = buscarProductoBinaria(codigo);    
    
if (producto == NULL) 
{ 
    printf(" Producto no encontrado.\n"); return; 
}
    printf("  Editando: %s (precio %.2f, stock %d, IVA %d%%)\n", producto->nombre, producto->precio, producto->stock, producto->iva);
    
    char nuevoNombre[TAM_NOMBRE];
    leerCadena("  Nuevo nombre (ENTER = mantener el actual): ", nuevoNombre, TAM_NOMBRE);
    if (strlen(nuevoNombre) > 0) strcpy(producto->nombre, nuevoNombre);
    producto->precio = redondear2(leerDineroPositivo("  Nuevo precio (USD): ")); 
    producto->stockMinimo = leerEnteroPositivo("  Nuevo stock minimo: ");
    producto->iva = leerIVA();                                               
    guardarProductos();                                                   
    printf(" Producto actualizado en el archivo binario.\n");
}
 
//FUNCION PARA ELIMINAR UN PRODUCTO DE LA LISTA Y DEL ARCHIVO:
void eliminarProducto(void) 
{
int codigo = leerEnteroPositivo("  Codigo del producto a eliminar: ");
Producto *actual = listaProductos, *anterior = NULL;
while (actual != NULL && actual->codigo != codigo) 
{     
    anterior = actual;
    actual = actual->siguiente;
}
    if (actual == NULL) 
    { 
        printf(" Producto no encontrado.\n"); 

    return; 
    }
    if (anterior == NULL) listaProductos = actual->siguiente; 
    else anterior->siguiente = actual->siguiente;             
    free(actual);                                           
    guardarProductos();                                    
    printf(" Producto eliminado del sistema y del archivo.\n");
}
 
//FUNCION PARA LISTAR TODOS LOS PRODUCTOS REGISTRADOS:
void listarProductos(void) 
{
    if (listaProductos == NULL) 
    { 
        printf(" No hay productos registrados.\n"); 
    return; 
    }
    printf("\n  %-8s %-25s %10s %8s %8s %6s\n", "Codigo", "Nombre", "Precio", "Stock", "Minimo", "IVA");
    printf("  ---------------------------------------------------------------------\n");
    Producto *producto;

    //BUCLE PARA IMPRIMIR CADA PRODUCTO DE LA LISTA:
    for (producto = listaProductos; producto != NULL; producto = producto->siguiente)
        printf("  %-8d %-25s %10.2f %8d %8d %5d%%\n", producto->codigo, producto->nombre, producto->precio, producto->stock, producto->stockMinimo, producto->iva);
}