# 🛒 POLI POS - Sistema de Facturación, Inventario y Caja

<p align="center">
  <img src="https://img.shields.io/badge/Lenguaje-C-00599C?style=for-the-badge&logo=c&logoColor=white" alt="Lenguaje C">
  <img src="https://img.shields.io/badge/Entorno-Consola-green?style=for-the-badge" alt="Consola">
  <img src="https://img.shields.io/badge/Estructura-Memoria_Dinámica-orange?style=for-the-badge" alt="Memoria Dinámica">
  <img src="https://img.shields.io/badge/Estado-Completado-success?style=for-the-badge" alt="Completado">
</p>

**POLI POS** es una aplicación interactiva de consola desarrollada íntegramente en **Lenguaje C**. Este sistema ofrece una solución integral para la gestión robusta de un punto de venta, unificando el control riguroso de inventarios, la facturación electrónica legal, la gestión detallada de clientes y los arqueos de caja, todo ello bajo estrictos estándares de estructuras dinámicas y persistencia en archivos.

---

## 🗺️ Arquitectura Visual y Funcional

Para comprender el flujo de datos y la integración de los módulos que componen POLI POS, hemos diseñado esta infografía técnica detallada. Muestra la interconexión entre la interfaz de terminal principal, los módulos de gestión (Clientes, Inventario, Ventas, Caja/Reportes) y la infraestructura técnica subyacente que garantiza la eficiencia y la persistencia del sistema.

<p align="center">
<img width="1408" height="768" alt="imagen para readme" src="https://github.com/user-attachments/assets/bcff13f6-bdf0-480d-8fba-1ab73ec368c8" />
</p>

---

## 📸 Demostración del Sistema

### 🖥️ Menú Principal
Aquí se aprecia la interfaz interactiva e intuitiva diseñada para el administrador y los cajeros, donde se centralizan todas las operaciones del sistema:

<p align="center">

<img width="627" height="272" alt="image" src="https://github.com/user-attachments/assets/e56ea601-b5d8-47a2-96b3-7b604298cc58" />

</p>

---

## 🚀 Características Clave del Proyecto

* **🧠 Memoria Dinámica Segura (`malloc`/`realloc`/`free`):** Estructura basada en listas enlazadas dinámicas para Productos, Clientes y Ventas, garantizando cero desperdicio de espacio en RAM y blindaje completo contra fugas de memoria (*Memory Leaks*).
* **💾 Persistencia de Datos:** Carga y guardado automático al iniciar/cerrar el programa usando archivos binarios (`.dat`) para inventarios e histórico de caja, y archivos de texto (`.txt`) para auditoría de facturas.
* **🛡️ Seguridad y Validación:** Rutinas avanzadas de entrada de consola que limpian el búfer y limitan la cantidad de caracteres admitidos para prevenir desbordamientos de búfer (*Buffer Overflow*).
* **📊 Algoritmos de Búsqueda y Ordenación:** Búsqueda binaria para la localización instantánea de productos y ordenación burbuja (*Bubble Sort*) para generar reportes dinámicos de clientes.
* **💵 Gestión de Caja:** Apertura obligatoria de caja para facturar, registro persistente de ingresos/egresos y desglose analítico de arqueo para un control financiero total.

---

## 🛠️ Instrucciones de Ejecución

### Compilar el proyecto
Abre tu terminal en la carpeta raíz donde están tus archivos `.c` y `.h` y ejecuta:
```bash
gcc main.c -o polipos
