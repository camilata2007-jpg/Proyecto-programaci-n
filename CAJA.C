//CAJA.C - MODULO DE CAJA [R16]
#include "poli_pos.h"
 
//Guarda el estado de la caja para que persista entre ejecuciones
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
        printf("  >> Aviso: no se pudo leer el estado previo de la caja.\n");
    fclose(archivo);
}
 
// APERTURA: solo pide el MONTO inicial q estrictamente positivo 
void abrirCaja(void) {
    if (caja.abierta) {
        printf("  >> La caja YA esta abierta con %.2f USD.\n", caja.montoApertura);
        return;
    }
    printf("\n--- APERTURA DE CAJA ---\n");
    caja.montoApertura = redondear2(leerDineroPositivo("  Monto de apertura (USD, mayor a 0): "));
    caja.idVentaInicio = siguienteIdVenta;  
    caja.abierta = 1;
    guardarCaja();                         
    printf("  >> Caja ABIERTA con %.2f USD.\n", caja.montoApertura);
}
 
//total en dolares de las ventas del turno (id >= idInicio).
//Caso base: NULL. Caso recursivo: aporte del nodo + total del resto
float totalVentasDesdeIdRec(Venta *nodo, int idInicio) {
    if (nodo == NULL) return 0.0f;                         
    float aporte = (nodo->id >= idInicio) ? nodo->total : 0.0f;
    return aporte + totalVentasDesdeIdRec(nodo->siguiente, idInicio);
}
 
// RECURSIVO: cuantas ventas tuvo el turno 
int contarVentasDesdeIdRec(Venta *nodo, int idInicio) {
    if (nodo == NULL) return 0;                             
    return ((nodo->id >= idInicio) ? 1 : 0)
           + contarVentasDesdeIdRec(nodo->siguiente, idInicio);
}
 
//CIERRE: muestra el CONSOLIDADO del turno por consola y lo respalda
void cerrarCaja(void) {
    if (!caja.abierta) { printf("  >> La caja no esta abierta.\n"); return; }
    printf("\n--- CIERRE DE CAJA ---\n");
    //el consolidado se calcula con subprogramas RECURSIVOS 
    int   ventasDelTurno   = contarVentasDesdeIdRec(listaVentas, caja.idVentaInicio);
    float totalDelTurno    = redondear2(totalVentasDesdeIdRec(listaVentas, caja.idVentaInicio));
    float efectivoEsperado = redondear2(caja.montoApertura + totalDelTurno);
    printf("  ========= CONSOLIDADO DE CIERRE =========\n");
    printf("  Monto de apertura        : %.2f USD\n", caja.montoApertura);
    printf("  Ventas del turno         : %d\n", ventasDelTurno);
    printf("  Total vendido            : %.2f USD\n", totalDelTurno);
    printf("  Efectivo esperado en caja: %.2f USD\n", efectivoEsperado);
    printf("  =========================================\n");
    //respaldo del consolidado en archivo de texto ("a" = anadir) 
    FILE *archivo = fopen(ARCH_CIERRES, "a");
    if (archivo != NULL) {
        fprintf(archivo, "CIERRE | Apertura: %.2f | Ventas: %d | Total: %.2f | Esperado: %.2f\n",
                caja.montoApertura, ventasDelTurno, totalDelTurno, efectivoEsperado);
        fclose(archivo);
        printf("  >> Consolidado guardado en %s\n", ARCH_CIERRES);
    }
    caja.abierta = 0;
    caja.montoApertura = 0.0f;
    caja.idVentaInicio = 0;
    guardarCaja();
    printf("  >> Caja CERRADA correctamente.\n");
}