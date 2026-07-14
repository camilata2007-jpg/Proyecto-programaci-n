//MODULO DE REPORTES AVANZADOS
#include "poli_pos.h"
 
/*SUBPROGRAMA RECURSIVO: suma los totales de las ventas cuya fecha
   cae dentro del rango. Caso base: lista vacia (NULL) devuelve 0.
   Caso recursivo: aporte de ESTE nodo + la suma del resto de la lista. */
float sumarVentasEnRangoRec(Venta *nodo, const char *desde, const char *hasta) {
    if (nodo == NULL) return 0.0f;                          /* caso base */
    float aporte = 0.0f;
    if (strcmp(nodo->fecha, desde) >= 0 && strcmp(nodo->fecha, hasta) <= 0)
        aporte = nodo->total;                               /* esta dentro del rango */
    return aporte + sumarVentasEnRangoRec(nodo->siguiente, desde, hasta);
}
 
/* Pregunta el destino del reporte: 1=consola, 2=archivo .txt.
   Devuelve stdout (la consola) o el archivo abierto; como ambos son FILE*,
   el mismo fprintf sirve para los dos destinos sin duplicar codigo. */
FILE *abrirDestinoReporte(const char *nombreArchivo, int *esArchivo) {
    printf("  Salida del reporte: 1) Consola  2) Exportar a %s\n", nombreArchivo);
    int opcion = leerEnteroEnRango("  Opcion: ", 1, 2);
    if (opcion == 2) {
        FILE *archivo = fopen(nombreArchivo, "w");        
        if (archivo != NULL) { *esArchivo = 1; return archivo; }
        printf("  >> No se pudo crear el archivo; se usara la consola.\n");
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
        printf("  >> La fecha final es menor a la inicial.\n");
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
 //funcion de ordenamiento descendente para el reporte de productos mas vendidos
void burbujaMasVendidos(ItemVendido arreglo[], int total) {
    for (int pasada = 0; pasada < total - 1; pasada++)
        for (int indice = 0; indice < total - 1 - pasada; indice++)
            if (arreglo[indice].unidades < arreglo[indice + 1].unidades) {
                ItemVendido temporal = arreglo[indice];     
                arreglo[indice] = arreglo[indice + 1];
                arreglo[indice + 1] = temporal;
            }
}
 
//PRODUCTOS MAS VENDIDOS (burbuja desc., consola o .txt) 
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
    if (usados == 0) { printf("  >> Aun no hay ventas.\n"); free(items); return; }
    burbujaMasVendidos(items, usados);                  
    int esArchivo = 0;
    FILE *salida = abrirDestinoReporte("reporte_mas_vendidos.txt", &esArchivo);
    fprintf(salida, "===== PRODUCTOS MAS VENDIDOS (burbuja descendente) =====\n");
    fprintf(salida, "%-4s %-8s %-25s %10s %12s\n", "Pos", "Codigo", "Producto", "Unidades", "Monto USD");
    for (int indice = 0; indice < usados; indice++)
        fprintf(salida, "%-4d %-8d %-25s %10d %12.2f\n",
                indice + 1, items[indice].codigo, items[indice].nombre,
                items[indice].unidades, redondear2(items[indice].monto));
    if (esArchivo) { fclose(salida); printf("  >> Reporte exportado a reporte_mas_vendidos.txt\n"); }
    free(items);                                            
}
 
//parte que ordena por stock bajo minimo y genera el reporte de stock bajo minimo
void burbujaPorCantidad(Producto *arreglo[], int total) {
    for (int pasada = 0; pasada < total - 1; pasada++)
        for (int indice = 0; indice < total - 1 - pasada; indice++)
            if (arreglo[indice]->stock > arreglo[indice + 1]->stock) {
                Producto *temporal = arreglo[indice];        
                arreglo[indice] = arreglo[indice + 1];
                arreglo[indice + 1] = temporal;
            }
}
 
//STOCK BAJO MINIMO, ordenado por CANTIDAD (burbuja). Consola o .txt. Es el unico reporte del Encargado de Inventario 
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
        printf("  >> Excelente: ningun producto esta bajo el stock minimo.\n");
        free(arreglo);
        return;
    }
    burbujaPorCantidad(arreglo, bajos);                   
    int esArchivo = 0;
    FILE *salida = abrirDestinoReporte("reporte_stock_bajo.txt", &esArchivo);
    fprintf(salida, "===== STOCK BAJO MINIMO =====\n");
    fprintf(salida, "%-8s %-25s %8s %8s %10s\n", "Codigo", "Producto", "Stock", "Minimo", "Faltante");
    for (int indice = 0; indice < bajos; indice++)
        fprintf(salida, "%-8d %-25s %8d %8d %10d\n",
                arreglo[indice]->codigo, arreglo[indice]->nombre,
                arreglo[indice]->stock, arreglo[indice]->stockMinimo,
                arreglo[indice]->stockMinimo - arreglo[indice]->stock);
    if (esArchivo) { fclose(salida); printf("  >> Reporte exportado a reporte_stock_bajo.txt\n"); }
    free(arreglo);
}
 
//SELECCION DESCENDENTE por monto: en cada vuelta BUSCA el cliente
//con el monto mas alto de lo que falta y lo coloca con UN solo intercambio 
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
 
//RANKING DE CLIENTES por monto (seleccion, consola o .txt) 
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