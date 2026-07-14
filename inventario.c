#include "poli_pos.h"
 
//FUNCION PARA RESPALDAR UN MOVIMIENTO DE INVENTARIO EN EL ARCHIVO BINARIO:
void respaldarMovimiento(Movimiento movimiento) 
{
    FILE *archivo = fopen(ARCH_INVENTARIO, "ab");
    if (archivo == NULL) 
    { 
        printf("  >> Error al abrir %s\n", ARCH_INVENTARIO); 
    return; 
    }
    fwrite(&movimiento, sizeof(Movimiento), 1, archivo);
    fclose(archivo);
}
 
//FUNCION PARA AJUSTAR EL STOCK DE UN PRODUCTO:
void ajustarStock(int *stock, int cantidad) 
{
    *stock = *stock + cantidad; 
}
 
//FUNCION PARA REGISTRAR UN MOVIMIENTO DE INVENTARIO:
void registrarMovimientoInventario(Producto *producto, const char *tipo, int cantidad, const char *fecha) 
{
    Movimiento movimiento;

    strcpy(movimiento.fecha, fecha);              
    strncpy(movimiento.tipo, tipo, sizeof(movimiento.tipo) - 1);

    movimiento.tipo[sizeof(movimiento.tipo) - 1] = '\0';
    movimiento.codigoProducto = producto->codigo;

    strncpy(movimiento.nombreProducto, producto->nombre, TAM_NOMBRE - 1);
    movimiento.nombreProducto[TAM_NOMBRE - 1] = '\0';
    movimiento.cantidad = cantidad;
    ajustarStock(&producto->stock, cantidad);      
    movimiento.stockResultante = producto->stock;
    respaldarMovimiento(movimiento);             
    guardarProductos();                        
}
 
//FUNCION PARA REGISTRAR UNA COMPRA:
void registrarCompra(void) 
{
    printf("\n--- REGISTRO DE COMPRA (ENTRADA DE INVENTARIO) ---\n");

    int codigo = leerEnteroPositivo("  Codigo del producto: ");
    Producto *producto = buscarProductoSecuencial(codigo);   

    if (producto == NULL) 
    { 
        printf("  >> Producto no encontrado.\n"); 

    return; 
    }
    int cantidad = leerEnteroPositivo("  Cantidad comprada: ");    
    char fecha[TAM_FECHA];
    leerFecha("  Fecha de la compra (AAAA-MM-DD): ", fecha);  
    registrarMovimientoInventario(producto, "COMPRA", cantidad, fecha);  
    printf("  >> Compra registrada. Nuevo stock de '%s': %d\n", producto->nombre, producto->stock);
}
 
//FUNCION PARA REGISTRAR UNA AJUSTE:
void registrarAjuste(void) 
{
    printf("\n--- AJUSTE DE STOCK ---\n");
    int codigo = leerEnteroPositivo("  Codigo del producto: ");
    
    Producto *producto = buscarProductoSecuencial(codigo);
    if (producto == NULL) 
    { 
        printf("  >> Producto no encontrado.\n"); 

    return; 
    }

    printf("  Stock actual de '%s': %d\n", producto->nombre, producto->stock);
    int cantidad;
    while (1) 
    {
        cantidad = leerEntero("  Cantidad de ajuste (+entrada / -salida, distinto de 0): ");
        if (cantidad == 0) 
        { 
            printf("  >> El ajuste no puede ser cero.\n"); 
        continue; 
        }
        if (producto->stock + cantidad < 0) 
        {     
            printf("  >> El ajuste dejaria el stock en negativo (stock actual: %d).\n", producto->stock);
        
        continue;
        }
        break;
    }
    char fecha[TAM_FECHA];
    leerFecha("  Fecha del ajuste (AAAA-MM-DD): ", fecha);
    registrarMovimientoInventario(producto, "AJUSTE", cantidad, fecha);
    printf("  >> Ajuste aplicado. Nuevo stock: %d\n", producto->stock);
}
 
//FUNCION PARA CONSULTAR EL HISTORICO DE INVENTARIO:
void consultarHistoricoInventario(void) 
{
    FILE *archivo = fopen(ARCH_INVENTARIO, "rb");
    if (archivo == NULL) 
    { 
        printf("  >> No existen movimientos registrados.\n"); 
    return; 
    }
    Movimiento movimiento;
    int totalMovimientos = 0;
    printf("\n  %-8s %-8s %-25s %10s %8s\n", "Tipo", "CodProd", "Producto", "Cantidad", "Stock");
    printf("  ----------------------------------------------------------------\n");
    while (fread(&movimiento, sizeof(Movimiento), 1, archivo) == 1) 
    {
        printf("  %-8s %-8d %-25s %10d %8d\n", movimiento.tipo, movimiento.codigoProducto, movimiento.nombreProducto, movimiento.cantidad, movimiento.stockResultante); 
        totalMovimientos++;
    }
    fclose(archivo);
    printf("  Total de movimientos: %d\n", totalMovimientos);
}