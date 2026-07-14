// MENUS POR ROL Y AUTENTICACION [R18][R20]
#include "poli_pos.h"
 
//ortal del CLIENTE: catalogo y su historial. Devuelve 0 (salir)
//o 7 (cambiar de usuario) para que main decida que hacer. 
int menuPortalCliente(void) {
    while (1) {
        printf("\n============ PORTAL DEL CLIENTE ============\n");
        printf("  Sesion: %s (%s)\n", clienteActual->nombre, clienteActual->cedula);
        printf("--------------------------------------------\n");
        printf(" 1) Ver catalogo de productos y precios\n");
        printf(" 2) Mis compras (historial)\n");
        printf(" 7) Cambiar de usuario\n");
        printf(" 0) Salir\n");
        int opcion = leerEnteroEnRango("Opcion: ", 0, 7);
        if (opcion == 1) listarProductos();         
        else if (opcion == 2) verMisCompras();
        else if (opcion == 0 || opcion == 7) return opcion;
        else printf("  >> Opcion invalida.\n");
    }
}
 
//PRODUCTOS: Admin y Encargado = CRUD completo | Cajero = SOLO listar 
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
    // Administrador y Encargado de Inventario: control total 
    while (1) {
        printf("\n===== PRODUCTOS (%s) =====\n", nombreRol(rol));
        printf(" 1) Registrar producto\n");
        printf(" 2) Listar productos\n");
        printf(" 3) Editar / actualizar producto\n");
        printf(" 4) Eliminar producto\n");
        printf(" 5) Buscar por codigo (busqueda binaria)\n");
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
            if (encontrado == NULL) printf("  >> No encontrado.\n");
            else printf("  Encontrado (binaria): %s | Precio %.2f | Stock %d | IVA %d%%\n",
                        encontrado->nombre, encontrado->precio,
                        encontrado->stock, encontrado->iva);
        }
    }
}
 
//INVENTARIO: Admin y Encargado (el Cajero ni ve este modulo)
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
 
//CLIENTES: Admin = registrar/buscar/listar/eliminar
//Cajero = SOLO registrar y buscar | Encargado = sin acceso
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
 
//VENTAS: Admin = registrar/listar/buscar por cliente
//  | Cajero = SOLO registrar | Encargado = sin acceso 
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
 
//REPORTES: Admin = los 4 reportes | Encargado = SOLO stock bajo
// | Cajero = sin acceso */
void menuReportes(int rol) {
    if (rol == ROL_INVENTARIO) {
        
        printf("\n===== REPORTES =====\n");
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
 
//CAJA: Admin y Cajero (el Encargado no maneja dinero) 
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
 
/* AUTENTICACION EN DOS PASOS:
   Paso 1: elegir tipo de usuario -> 1) CLIENTE   2) PERSONAL
   Paso 2: - CLIENTE : entra SOLO con su cedula validada (sin contrasena)
           - PERSONAL: entra con contrasena; la clave define su ROL */
int iniciarSesion(void) {
    printf("=========================================================\n");
    printf("   POLI POS - Sistema de Punto de Venta e Inventario v2\n");
    printf("=========================================================\n");
    printf(" 1) CLIENTE (ingresa con su cedula)\n");
    printf(" 2) PERSONAL (ingresa con contrasena)\n");
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
                printf(" Cedula no registrada. Registrese a continuacion:\n");
                clienteActual = registrarCliente();
                if (clienteActual == NULL) continue;
            }
            break;
        }
        printf("\n Bienvenido %s, entraste como: CLIENTE\n", clienteActual->nombre);
        return ROL_CLIENTE;
    }
 
    //PERSONAL: la contrasena define el rol; 3 intentos y se bloquea
    char clave[40];
    int intentosRestantes = 3;
    while (intentosRestantes > 0) {
        leerCadena("Ingrese su contrasena: ", clave, sizeof(clave));
        if (strcmp(clave, CLAVE_ADMIN) == 0) {
            printf("\n Bienvenido, entraste como: ADMINISTRADOR\n");
            return ROL_ADMIN;
        } else if (strcmp(clave, CLAVE_CAJERO) == 0) {
            printf("\n Bienvenido, entraste como: CAJERO\n");
            return ROL_CAJERO;
        } else if (strcmp(clave, CLAVE_INVENTARIO) == 0) {
            printf("\n Bienvenido, entraste como: ENCARGADO DE INVENTARIO\n");
            return ROL_INVENTARIO;
        }
        intentosRestantes--;
        printf(" Contrasena incorrecta. Intentos restantes: %d\n", intentosRestantes);
    }
    printf(" Acceso bloqueado por seguridad. El programa terminara.\n");
    exit(EXIT_FAILURE);
}