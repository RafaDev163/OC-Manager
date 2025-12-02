#ifndef procesosOC_H
#define procesosOC_H

#include <stddef.h>   // size_t
#include <time.h>     // time_t
#include <stdint.h>  // int32_t, uint32_t

// Constantes
#define MAX_SIZE_NOMBRE_PROVEEDOR 100
#define MAX_SIZE_NOMBRE_ETIQUETADOR 50

//Codigos de error

#define OC_OK                 0
#define OC_ERR_PARAM         -1
#define OC_ERR_MEMORIA       -2
#define OC_ERR_ESTADO        -3
#define OC_ERR_TIEMPO        -4
#define OC_ERR_DUPLICADA     -5
#define OC_ERR_NO_INICIADO   -6
#define OC_ERR_YA_FINALIZADO -7
#define OC_ERR_IO            -8
#define OC_ERR_LECTURA       -9

// Estructuras para la ejecucion del programa (memoria)
typedef enum {
    OC_PENDIENTE = 0,
    OC_ETIQUETANDO,
    OC_ETIQUETADA
} EstadoProcesoOC;

typedef struct ProcesoOC {
    int num_OC;
    char nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR];
    char nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR];
    size_t cant_productos;
    time_t inicio;  // Marca de tiempo de inicio
    time_t fin;     // Marca de tiempo de fin
    EstadoProcesoOC estado;
    struct ProcesoOC *sig;
} ProcesoOC;

// Estructura para el almacenamiento en disco de los datos
typedef struct{
    int32_t estado;
    int num_OC;
    char nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR];
    char nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR];
    size_t cant_productos;
    time_t inicio;
    time_t fin;    
} ProcesoOC_on_Disk;

//CRUD de procesos de ordenes de compra
int agregarProcesoOC(ProcesoOC **lista, int num_OC, const char *nombre_prov, const char *nombre_etiq, size_t cant_productos);
int iniciarEtiquetadoOC(ProcesoOC *proceso);
int finalizarEtiquetadoOC(ProcesoOC *proceso);
void listarProcesosOC(const ProcesoOC *lista);

// Funciones de utilidad
void limpiar_buffer();
void liberarListaProcesosOC(ProcesoOC **lista);
ProcesoOC* buscarProcesoOC(ProcesoOC *lista, int num_OC);

/* Almacenamiento en disco .bin */
int guardarProcesosOCEnDisco(const char *nombre_archivo, const ProcesoOC *lista);
int leerProcesosOCDesdeDisco(const char *nombre_archivo, ProcesoOC **out_lista, uint32_t *out_count);



#endif