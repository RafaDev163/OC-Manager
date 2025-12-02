#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "procesosOC.h"
    
    

void limpiar_buffer(){
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * Libera la memoria de la lista enlazada de procesos OC.
 * 
 * IMPORTANTE: Al finalizar, el puntero de la lista será establecido en NULL.
 * Asegúrese de que el código que llama a esta función no utilice el puntero
 * sin verificar que ha sido puesto en NULL.
 *
 * @param lista Puntero al puntero de la lista de procesos OC a liberar.
 */
void liberarListaProcesosOC(ProcesoOC **lista) {
    ProcesoOC *actual = *lista;

    while (actual) {
        ProcesoOC *siguiente = actual->sig;
        free(actual);
        actual = siguiente;
    }

    *lista = NULL;
}

/**
 * Agrega un nuevo proceso de Orden de Compra (OC) a la lista enlazada.
 * 
 * @param lista Puntero al puntero de la lista de procesos OC.
 * @param num_OC Número de la orden de compra.
 * @param nombre_prov Nombre del proveedor.
 * @param nombre_etiq Nombre del etiquetador.
 * @param cant_productos Cantidad de productos en la orden.
 * @return 0 en caso de éxito, -1 en caso de error de parámetros o memoria.
 */
int agregarProcesoOC(ProcesoOC **lista, int num_OC, const char *nombre_prov, const char *nombre_etiq, size_t cant_productos) {
    if(!lista || !nombre_prov || !nombre_etiq) {
        return OC_ERR_PARAM; // Error de parámetros
    }

    //Verificar si ya existe una OC con el mismo número
      if (buscarProcesoOC(*lista, num_OC) != NULL) {
        return OC_ERR_DUPLICADA; // OC duplicada
    }

    ProcesoOC *nuevo_proceso = (ProcesoOC *)malloc(sizeof(ProcesoOC));
    if (!nuevo_proceso) {
        perror("Error al asignar memoria para nuevo proceso OC");
        return OC_ERR_MEMORIA; // Error de memoria
    }

    nuevo_proceso->num_OC = num_OC;
    // Copia segura con truncado
    // snprintf garantiza terminación en '\0'
    snprintf(nuevo_proceso->nombre_prov, MAX_SIZE_NOMBRE_PROVEEDOR, "%s", nombre_prov);
    // Copia segura con truncado
    // snprintf garantiza terminación en '\0'
    snprintf(nuevo_proceso->nombre_etiq, MAX_SIZE_NOMBRE_ETIQUETADOR, "%s", nombre_etiq);
    
    nuevo_proceso->cant_productos = cant_productos;
    nuevo_proceso->inicio = 0;
    nuevo_proceso->fin = 0;
    nuevo_proceso->estado = OC_PENDIENTE;
    nuevo_proceso->sig = *lista;
    *lista = nuevo_proceso;

    return 0; // Éxito
}

int iniciarEtiquetadoOC(ProcesoOC *proceso) {
    if (!proceso) {
        return OC_ERR_PARAM; // Error de parámetros
    }

    if (proceso->estado != OC_PENDIENTE) {
        return OC_ERR_ESTADO; // Proceso no está en estado pendiente
    }

    proceso->inicio = time(NULL);
    proceso->estado = OC_ETIQUETANDO;

    return 0; // Éxito
}

int finalizarEtiquetadoOC(ProcesoOC *proceso) {
    if (!proceso) {
        return OC_ERR_PARAM; // Error de parámetros
    }

    if (proceso->estado != OC_ETIQUETANDO) {
        return OC_ERR_ESTADO; // Proceso no está en estado etiquetando
    }

    proceso->fin = time(NULL);
    proceso->estado = OC_ETIQUETADA;

    return 0; // Éxito
}

ProcesoOC* buscarProcesoOC(ProcesoOC *lista, int num_OC) {
    while (lista) {
        if (lista->num_OC == num_OC) {
            return lista; // encontrado
        }
        lista = lista->sig;
    }
    return NULL; // no encontrado
}

void listarProcesosOC(const ProcesoOC *lista) {
    const ProcesoOC *actual = lista;
    printf("Listado de Procesos de Ordenes de Compra:\n");
    printf("------------------------------------------------------------\n");
    while (actual) {
        char inicio_str[26];
        char fin_str[26];
        strcpy(inicio_str, "N/A");
        strcpy(fin_str, "N/A");

        if (actual->inicio != 0) {
            ctime_s(inicio_str, sizeof(inicio_str), &actual->inicio);
            inicio_str[strcspn(inicio_str, "\n")] = 0;
        }
        if (actual->fin != 0) {
            ctime_s(fin_str, sizeof(fin_str), &actual->fin);
            fin_str[strcspn(fin_str, "\n")] = 0;
        }

        const char *estado_str;
        switch (actual->estado) {
            case OC_PENDIENTE:
                estado_str = "PENDIENTE";
                break;
            case OC_ETIQUETANDO:
                estado_str = "ETIQUETANDO";
                break;
            case OC_ETIQUETADA:
                estado_str = "ETIQUETADA";
                break;
            default:
                estado_str = "DESCONOCIDO";
                break;
        }

        printf("OC Nro: %d | Proveedor: %s | Etiquetador: %s | Cantidad Productos: %zu | Inicio: %s | Fin: %s | Estado: %s\n",
               actual->num_OC,
               actual->nombre_prov,
               actual->nombre_etiq,
               actual->cant_productos,
               inicio_str,
               fin_str,
               estado_str);

        actual = actual->sig;
    }
    printf("------------------------------------------------------------\n");
}


/* * Guarda la lista de procesos OC en un archivo binario.
 * 
 * @param nombre_archivo Nombre del archivo donde se guardarán los datos.
 * @param lista Puntero a la lista de procesos OC.
 * @return 0 en caso de éxito, código de error en caso contrario.
 */ 

int guardarProcesosOCEnDisco(const char *nombre_archivo, const ProcesoOC *lista) {
    if (!nombre_archivo) {
        return OC_ERR_PARAM; // Error de parámetros
    }

    FILE *archivo = fopen(nombre_archivo, "wb");
    if (!archivo) {
        perror("Error al abrir archivo para escritura");
        return OC_ERR_IO;
    }

    // Contar la cantidad de procesos en la lista
    uint32_t count = 0;
    for(const ProcesoOC *temp = lista; temp; temp = temp->sig) {
        count++;
    }
    // Escribir la cantidad de procesos al inicio del archivo
    if(fwrite(&count, sizeof(count), 1, archivo) != 1) {
        perror("Error al escribir la cantidad de procesos en el archivo");
        fclose(archivo);
        return OC_ERR_IO;
    }
    // Escribir cada proceso en el archivo
    for(const ProcesoOC *actual = lista; actual; actual = actual->sig) {
        ProcesoOC_on_Disk registro_disk;
        registro_disk.estado = (int32_t)actual->estado;
        registro_disk.num_OC = actual->num_OC;
        strncpy(registro_disk.nombre_prov, actual->nombre_prov, MAX_SIZE_NOMBRE_PROVEEDOR);
        registro_disk.nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR - 1] = '\0';
        strncpy(registro_disk.nombre_etiq, actual->nombre_etiq, MAX_SIZE_NOMBRE_ETIQUETADOR);
        registro_disk.nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR - 1] = '\0';
        registro_disk.cant_productos = actual->cant_productos;
        registro_disk.inicio = actual->inicio;
        registro_disk.fin = actual->fin;

        if (fwrite(&registro_disk, sizeof(ProcesoOC_on_Disk), 1, archivo) != 1) {
            perror("Error al escribir un registro en el archivo");
            fclose(archivo);
            return OC_ERR_IO;
        }
    }
    fclose(archivo);
    return OC_OK;

}

/* Lee la lista de procesos OC desde un archivo binario.
 * 
 * @param nombre_archivo Nombre del archivo desde donde se leerán los datos.
 * @param out_lista Puntero al puntero donde se almacenará la lista leída.
 * @param out_count Puntero donde se almacenará la cantidad de procesos leídos.
 * @return 0 en caso de éxito, código de error en caso contrario.
 */
int leerProcesosOCDesdeDisco(const char *nombre_archivo, ProcesoOC **out_lista, uint32_t *out_count) {
    if( !nombre_archivo || !out_lista || !out_count) {
        return OC_ERR_PARAM; // Error de parámetros
    }

    *out_lista = NULL;
    *out_count = 0;

    FILE *archivo = fopen(nombre_archivo, "rb");
    if (!archivo) {
        if (errno == ENOENT) {
            // El archivo no existe: caso normal en el primer arranque
            return OC_OK;
        }
        perror("Error al abrir archivo para lectura");
        return OC_ERR_IO;
    }
    
    uint32_t count = 0;
    if (fread(&count, sizeof count, 1, archivo) != 1) {
        if (feof(archivo)) {
            // Archivo vacío → lo tratamos como sin procesos
            fclose(archivo);
            return OC_OK;
        }
        perror("Error al leer la cantidad de procesos del archivo");
        fclose(archivo);
        return OC_ERR_IO;
    }
    ProcesoOC *lista = NULL, *ultimo = NULL;
    for (uint32_t i = 0; i < count; i++) {
        ProcesoOC_on_Disk registro_disk;
        if (fread(&registro_disk, sizeof(ProcesoOC_on_Disk), 1, archivo) != 1) {
            perror("Error al leer un registro del archivo");
            liberarListaProcesosOC(&lista);
            fclose(archivo);
            return OC_ERR_IO;
        }

        ProcesoOC *nuevo_proceso = malloc(sizeof *nuevo_proceso);
        if (!nuevo_proceso) {
            perror("Error al asignar memoria para nuevo proceso OC");
            liberarListaProcesosOC(&lista);
            fclose(archivo);
            return OC_ERR_MEMORIA;
        }

        nuevo_proceso->num_OC = registro_disk.num_OC;
        strncpy(nuevo_proceso->nombre_prov, registro_disk.nombre_prov, MAX_SIZE_NOMBRE_PROVEEDOR);
        nuevo_proceso->nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR - 1] = '\0';
        strncpy(nuevo_proceso->nombre_etiq, registro_disk.nombre_etiq, MAX_SIZE_NOMBRE_ETIQUETADOR);
        nuevo_proceso->nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR - 1] = '\0';
        nuevo_proceso->cant_productos = registro_disk.cant_productos;
        nuevo_proceso->inicio = registro_disk.inicio;
        nuevo_proceso->fin = registro_disk.fin;
        nuevo_proceso->estado = (EstadoProcesoOC)registro_disk.estado;
        nuevo_proceso->sig = NULL;

        if(!lista) {
            lista = ultimo = nuevo_proceso;
        } else {
            ultimo->sig = nuevo_proceso;
            ultimo = nuevo_proceso;
        }
    }
    fclose(archivo);
    *out_lista = lista;
    *out_count = count;
    return OC_OK;
}
   
