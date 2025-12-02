# oc manager — v1.0.0
**Módulo de Gestión y Persistencia de Procesos OC**  
**Autor:** Rafael Garcia Torres
**Lenguaje:** C (Estándar C18)  
**Estado:** Versión estable inicial

---

## 1. Descripción General

`oc manager` es un módulo desarrollado en C cuyo propósito es administrar la información asociada a los **Procesos OC** (Ordenes de compra) dentro de sistemas orientados al análisis de eficiencia operativa. Esta primera versión estable proporciona funcionalidades esenciales para almacenar, consultar y mantener registros operativos tanto en memoria como en archivo binario, con un diseño modular y extensible para futuras versiones del sistema.

---

## 2. Características Principales

- **Gestión dinámica de datos:** manejo de estructuras `ProcesoOC` en memoria.  
- **Persistencia en disco:** lectura y escritura en formato binario.  
- **Interfaz interactiva:** menú sencillo con limpieza automática de pantalla.  
- **Diseño modular:** separación clara entre lógica del módulo y la capa de interacción.  
- **Validación y seguridad:** manejo básico de errores y comprobación de parámetros.

---

## 3. Instalación y Compilación

### Requisitos
- GCC o Clang  
- Make  
- Sistema tipo POSIX (Linux, macOS, FreeBSD o MSYS2 en Windows)

### Compilación
- bash
- make clean
- make

---


## 4. Ejecución 

./oc_manager


Al ejecutarse, el programa generará automáticamente el archivo binario:

bin/procesos_oc.bin

./oc_manager


Al ejecutarse, el programa generará automáticamente el archivo binario:

bin/procesos_oc.bin

## 5. Uso del Sistema

El programa presenta un menú interactivo que permite:

Registrar nuevos procesos OC

Mostrar los procesos almacenados

Guardar datos en disco

Cargar información previamente guardada

Salir de la aplicación

Cada opción limpia la pantalla para ofrecer una interfaz ordenada y clara.

## 6. Alcance de esta Versión

Esta versión establece la base para versiones posteriores del proyecto, proporcionando:

Persistencia confiable

API simple y clara

Interfaz interactiva utilizable

Estructura modular apta para expansión



