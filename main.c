#include "poli_pos.h"
Producto  *listaProductos = NULL;   
Cliente   *listaClientes  = NULL;   
Venta     *listaVentas    = NULL;   
EstadoCaja caja           = {0, 0.0f, 0};   
Cliente   *clienteActual  = NULL;   
int siguienteIdVenta      = 1;      
 
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
            printf("\n>> Gracias por usar POLI POS. Datos guardados.\n");
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
 
        /* [R18] Doble proteccion: si escriben el numero de un modulo oculto,
           tambien se bloquea con el mensaje de acceso denegado */
        if (opcion >= 1 && opcion <= 6 && !rolTieneAcceso(rol, opcion)) {
            accesoDenegado();
            continue;
        }
 
        /* main solo INVOCA funciones: toda la logica vive en los modulos */
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