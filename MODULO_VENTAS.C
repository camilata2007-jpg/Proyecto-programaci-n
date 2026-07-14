//MODULO DE VENTAS
#include "poli_pos.h"
 
//Guarda todas las ventas en el archivo binario
void guardarVentas(void) {
    FILE *archivo = fopen(ARCH_VENTAS, "wb");
    if (archivo == NULL) { printf("  Error al abrir %s\n", ARCH_VENTAS); return; }
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)
        fwrite(venta, sizeof(Venta), 1, archivo);
    fclose(archivo);
}
 
//ee el archivo y reconstruye la lista dinamica de ventas 
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
 
//Imprime una venta en una sola linea (para listados y busquedas)
void imprimirVenta(const Venta *venta) {
    printf("  #%-4d %s | %s (%s) | %s x%d | Sub: %.2f Desc: %.2f IVA: %.2f | TOTAL: %.2f\n",
           venta->id, venta->fecha, venta->nombreCliente, venta->cedulaCliente,
           venta->nombreProducto, venta->cantidad, venta->subtotal,
           venta->descuento, venta->valorIVA, venta->total);
}
 
//FACTURA COMPLETA MOSTRADA EN CONSOLA (evidencia del sistema):
//desglosa cliente, producto, subtotal, descuento, IVA y total a pagar 
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
 
//REGISTRO DE VENTA 
void registrarVenta(void) {
    if (!caja.abierta) {        
        printf("\n  >> La CAJA esta CERRADA. Debe abrir caja antes de vender.\n");
        return;
    }
    printf("\n--- REGISTRO DE VENTA ---\n");
    // 1. Cliente (vinculacion cliente-venta) 
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula del cliente: ", cedula, TAM_CEDULA);
    Cliente *cliente = buscarClientePorCedula(cedula);       
    if (cliente == NULL) {
        printf("  >> Cliente no registrado. Registrelo primero:\n");
        cliente = registrarCliente();
        if (cliente == NULL) return;
    }
    
    int codigo = leerEnteroPositivo("  Codigo del producto: ");
    Producto *producto = buscarProductoBinaria(codigo);
    if (producto == NULL) { printf("  >> Producto no encontrado.\n"); return; }
   
    printf("  Producto: %s | Precio: %.2f | Stock disponible: %d\n",
           producto->nombre, producto->precio, producto->stock);
    if (producto->stock <= 0) { printf("  >> SIN STOCK disponible. Venta cancelada.\n"); return; }
    int cantidad;
    while (1) {
        cantidad = leerEnteroPositivo("  Cantidad a vender: ");  
        if (cantidad <= producto->stock) break;    
        printf("  >> Stock insuficiente (disponible: %d).\n", producto->stock);
    }
    //fecha de la venta q ingresa el user
    char fechaVenta[TAM_FECHA];
    leerFecha("  Fecha de la venta (AAAA-MM-DD): ", fechaVenta);
    
    float porcentajeDescuento;
    while (1) {
        porcentajeDescuento = leerFlotante("  Descuento % (0 a 100): ");
        if (porcentajeDescuento >= 0.0f && porcentajeDescuento <= 100.0f) break;
        printf(" Descuento invalido: debe estar entre 0 y 100.\n");
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
    float baseImponible   = venta->subtotal - venta->descuento;  
    
    venta->valorIVA       = redondear2(baseImponible * producto->iva / 100.0f);
    venta->total          = redondear2(baseImponible + venta->valorIVA);
    if (venta->total <= 0.0f) {   
        printf(" Total invalido (%.2f). Venta cancelada.\n", venta->total);
        free(venta);
        return;
    }
    //Descargar stock con el subprograma por referencia
    ajustarStock(&producto->stock, -cantidad);
    guardarProductos();                                     
    venta->siguiente = listaVentas;
    listaVentas = venta;
    guardarVentas();
   
    imprimirFactura(venta);
    printf(" Venta registrada. Stock restante de '%s': %d\n",
           producto->nombre, producto->stock);
}
 
//Cuenta las ventas (para dimensionar arreglos de reportes [R13])
int contarVentas(void) {
    int cantidad = 0;
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente) cantidad++;
    return cantidad;
}
 
//busqueda de ventas por cedula del cliente
void buscarVentasPorCliente(void) {
    char cedula[TAM_CEDULA];
    leerCadena("  Cedula del cliente: ", cedula, TAM_CEDULA);
    int encontradas = 0;
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)   /* [R10] */
        if (strcmp(venta->cedulaCliente, cedula) == 0) {
            imprimirVenta(venta);
            encontradas++;
        }
    if (!encontradas) printf(" No hay ventas para ese cliente.\n");
}
 
//Listado completo de ventas
void listarVentas(void) {
    if (listaVentas == NULL) { printf(" No hay ventas registradas.\n"); return; }
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)
        imprimirVenta(venta);
}
 
// "Mis compras" del portal del cliente: busqueda secuencial de las ventas que pertenecen al cliente con sesion activa 
void verMisCompras(void) {
    int encontradas = 0;
    printf("\n--- MIS COMPRAS (%s) ---\n", clienteActual->nombre);
    Venta *venta;
    for (venta = listaVentas; venta != NULL; venta = venta->siguiente)  
        if (strcmp(venta->cedulaCliente, clienteActual->cedula) == 0) {
            imprimirVenta(venta);
            encontradas++;
        }
    if (!encontradas) printf(" Aun no tiene compras registradas.\n");
}