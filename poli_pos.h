#ifndef POLI_POS_H          
#define POLI_POS_H
 
//Librerias 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
//Constantes
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

//Estructuras
//Productos
typedef struct Producto {
    int   codigo;
    char  nombre[TAM_NOMBRE];
    float precio;                
    int   stock;                 
    int   stockMinimo;           
    int   iva;                   
    struct Producto *siguiente;  
} Producto;
 
//Cliente
typedef struct Cliente {
    char cedula[TAM_CEDULA];     
    char nombre[TAM_NOMBRE];
    struct Cliente *siguiente;   
} Cliente;
 
//Venta
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
 
//Movimiento
typedef struct Movimiento {
    char fecha[TAM_FECHA];      
    char tipo[16];              
    int  codigoProducto;
    char nombreProducto[TAM_NOMBRE];
    int  cantidad;
    int  stockResultante;
} Movimiento;
 
//Estado de Caja
typedef struct EstadoCaja {
    int   abierta;
    float montoApertura;
    int   idVentaInicio;         
} EstadoCaja;
 
// ItenmVendido
typedef struct ItemVendido {
    int   codigo;
    char  nombre[TAM_NOMBRE];
    int   unidades;
    float monto;
} ItemVendido;
 
//Cliente ingresado
typedef struct ClienteRank {
    char  cedula[TAM_CEDULA];
    char  nombre[TAM_NOMBRE];
    float monto;
    int   compras;
} ClienteRank;
 
//Variables globales
extern Producto  *listaProductos;   
extern Cliente   *listaClientes;    
extern Venta     *listaVentas;      
extern EstadoCaja caja;             
extern Cliente   *clienteActual;    
extern int        siguienteIdVenta; 
 
//Prototipos hutilidades
void  limpiarBuffer(void);
void  leerCadena(const char *mensaje, char *destino, int tamanio);
int   leerEntero(const char *mensaje);
int   leerEnteroPositivo(const char *mensaje);            
int   leerEnteroEnRango(const char *mensaje, int minimo, int maximo);
float leerFlotante(const char *mensaje);
float leerDineroPositivo(const char *mensaje);           
float redondear2(float valor);                            
int   esFechaValida(const char *fecha);
void  leerFecha(const char *mensaje, char *destino);     
int   validarCedulaEcuatoriana(const char *cedula);       
int   leerIVA(void);                                      
const char *nombreRol(int rol);                           
void  accesoDenegado(void);                               
int   rolTieneAcceso(int rol, int modulo);               
void  liberarListas(void);                                
 
//Prototipo productos
void      guardarProductos(void);                        
void      cargarProductos(void);                          
Producto *buscarProductoSecuencial(int codigo);           
int       contarProductos(void);
Producto *buscarProductoBinaria(int codigoBuscado);       
void      registrarProducto(void);
void      editarProducto(void);
void      eliminarProducto(void);
void      listarProductos(void);
 
//Prototipo Movimientos
void respaldarMovimiento(Movimiento movimiento);          
void ajustarStock(int *stock, int cantidad);              
void registrarMovimientoInventario(Producto *producto, const char *tipo,
                                   int cantidad, const char *fecha);  
void registrarCompra(void);                              
void registrarAjuste(void);                               
void consultarHistoricoInventario(void);                  
 
/* ========================= PROTOTIPOS: clientes.c ============================= */
void     guardarClientes(void);                           
void     cargarClientes(void);                            
void     mostrarCliente(Cliente cliente);                
int      cedulaYaRegistrada(char cedula[TAM_CEDULA]);    
Cliente *buscarClientePorCedula(const char *cedula);      
Cliente *registrarCliente(void);                          
void     buscarClienteMenu(void);
void     listarClientes(void);
void     eliminarCliente(void);
 

void guardarVentas(void);                               
void cargarVentas(void);                                  
void imprimirVenta(const Venta *venta);
void imprimirFactura(const Venta *venta);                 
void registrarVenta(void);                                
int  contarVentas(void);
void buscarVentasPorCliente(void);                        
void listarVentas(void);
void verMisCompras(void);                                 
 

float sumarVentasEnRangoRec(Venta *nodo, const char *desde, const char *hasta); 
FILE *abrirDestinoReporte(const char *nombreArchivo, int *esArchivo);  
void  reporteVentasPorRango(void);                        
void  burbujaMasVendidos(ItemVendido arreglo[], int total);      
void  reporteProductosMasVendidos(void);                  
void  burbujaPorCantidad(Producto *arreglo[], int total);        
void  reporteStockBajo(void);                             
void  seleccionRanking(ClienteRank arreglo[], int total);        
void  reporteRankingClientes(void);                       
 

void  guardarCaja(void);
void  cargarCaja(void);
void  abrirCaja(void);
float totalVentasDesdeIdRec(Venta *nodo, int idInicio);   
int   contarVentasDesdeIdRec(Venta *nodo, int idInicio);  
void  cerrarCaja(void);
 

int  menuPortalCliente(void);
void menuProductos(int rol);
void menuInventario(int rol);
void menuClientes(int rol);
void menuVentas(int rol);
void menuReportes(int rol);
void menuCaja(int rol);
int  iniciarSesion(void);
 
#endif 