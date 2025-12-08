#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "procesosOC.h"
    
// helpers de conversion disco <-> memoria
static void disk_v1_0_0_to_mem(const ProcesoOC_on_Disk *disk, ProcesoOC *mem){
    mem->num_OC = disk->num_OC;
    strncpy(mem->nombre_prov, disk->nombre_prov, MAX_SIZE_NOMBRE_PROVEEDOR);
    mem->nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR - 1] = '\0';
    strncpy(mem->nombre_etiq, disk->nombre_etiq, MAX_SIZE_NOMBRE_ETIQUETADOR);
    mem->nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR - 1] = '\0';
    mem->cant_productos = disk->cant_productos;
    mem->inicio = disk->inicio;
    mem->fin = disk->fin;
    mem->estado = (EstadoProcesoOC)disk->estado;
    mem->fecha_surtido = 0; // No disponible en v1.0.0
}

static void disk_v1_0_1_to_mem(const ProcesoOC_on_Disk_v1_0_1 *disk, ProcesoOC *mem){
    mem->num_OC = disk->num_OC;
    strncpy(mem->nombre_prov, disk->nombre_prov, MAX_SIZE_NOMBRE_PROVEEDOR);
    mem->nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR - 1] = '\0';
    strncpy(mem->nombre_etiq, disk->nombre_etiq, MAX_SIZE_NOMBRE_ETIQUETADOR);
    mem->nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR - 1] = '\0';
    mem->cant_productos = disk->cant_productos;
    mem->inicio = disk->inicio;
    mem->fin = disk->fin;
    mem->estado = (EstadoProcesoOC)disk->estado;
    mem->fecha_surtido = disk->fecha_surtido;
}

static void mem_to_disk_v1_0_1(const ProcesoOC *mem, ProcesoOC_on_Disk_v1_0_1 *disk){
    disk->num_OC = mem->num_OC;
    strncpy(disk->nombre_prov, mem->nombre_prov, MAX_SIZE_NOMBRE_PROVEEDOR);
    disk->nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR - 1] = '\0';
    strncpy(disk->nombre_etiq, mem->nombre_etiq, MAX_SIZE_NOMBRE_ETIQUETADOR);
    disk->nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR - 1] = '\0';
    disk->cant_productos = mem->cant_productos;
    disk->inicio = mem->inicio;
    disk->fin = mem->fin;
    disk->estado = (int32_t)mem->estado;
    disk->fecha_surtido = mem->fecha_surtido;
}

// helper para listar todos los procesos OC
static void imprimirProcesoOC(const ProcesoOC *actual) {
    char inicio_str[26];
    char fin_str[26];
    char fecha_surtido_str[26];

    strcpy(inicio_str, "N/A");
    strcpy(fin_str, "N/A");
    strcpy(fecha_surtido_str, "N/A");

    if (actual->inicio != 0) {
        ctime_s(inicio_str, sizeof(inicio_str), &actual->inicio);
        inicio_str[strcspn(inicio_str, "\n")] = 0;
    }
    if (actual->fin != 0) {
        ctime_s(fin_str, sizeof(fin_str), &actual->fin);
        fin_str[strcspn(fin_str, "\n")] = 0;
    }
    if (actual->fecha_surtido != 0) {
        struct tm tm_surtido;
        if (localtime_s(&tm_surtido, &actual->fecha_surtido) == 0) {
            if (strftime(fecha_surtido_str,
                         sizeof(fecha_surtido_str),
                         "%d/%m/%Y",
                         &tm_surtido) == 0) {
                strcpy(fecha_surtido_str, "FECHA_ERR");
            }
        } else {
            strcpy(fecha_surtido_str, "FECHA_ERR");
        }
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
        case OC_SURTIDA:
            estado_str = "SURTIDA";
            break;
        default:
            estado_str = "DESCONOCIDO";
            break;
    }

    printf("OC Nro: %d | Proveedor: %s | Etiquetador: %s | Cantidad Productos: %zu\n"
           "  Inicio: %s | Fin: %s | Estado: %s | Fecha Surtido: %s\n",
           actual->num_OC,
           actual->nombre_prov,
           actual->nombre_etiq,
           actual->cant_productos,
           inicio_str,
           fin_str,
           estado_str,
           fecha_surtido_str);
}



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

int calcular_fecha_surtido(time_t fecha_cambio_surtido){
    //lunes de la semana siguiente
    struct tm *tm_info = localtime(&fecha_cambio_surtido);
    int w = tm_info->tm_wday; // 0=Domingo, 1=Lunes, ..., 6=Sábado
    int dias = 7 -((w + 6 )%7);
    if (dias == 0){
        dias = 7; // Si es lunes, sumar 7 días para el próximo lunes
    }
    return dias;
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
    nuevo_proceso->fecha_surtido = 0;
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

int surtirOC(ProcesoOC *proceso){
    if (!proceso) {
        return OC_ERR_PARAM; // Error de parámetros
    }

    if (proceso->estado != OC_ETIQUETADA) {
        return OC_ERR_ESTADO; // Proceso no está en estado etiquetada
    }
    time_t ahora = time(NULL);
    int dias_para_surtido = calcular_fecha_surtido(ahora);
    proceso->fecha_surtido = ahora + (dias_para_surtido * 24 * 60 * 60);
    proceso->estado = OC_SURTIDA;

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
    if (!actual) {
        printf("No hay procesos registrados.\n");
        printf("------------------------------------------------------------\n");
        return;
    }

    while (actual) {
        imprimirProcesoOC(actual);
        actual = actual->sig;
    }

    printf("------------------------------------------------------------\n");
}

void listarProcesosOCFiltrado(const ProcesoOC *lista, int usar_filtro_oc, int num_oc_filtro, int usar_filtro_estado, EstadoProcesoOC estado_filtro){
    const ProcesoOC *actual = lista;
    int encontrados = 0;
    
    printf("Listado de Procesos de Ordenes de Compra (Filtrado):\n");
    printf("------------------------------------------------------------\n");
    if (!actual) {
        printf("No hay procesos registrados.\n");
        printf("------------------------------------------------------------\n");
        return;
    }

    while (actual) {
        int coincide = 1;

        if (usar_filtro_oc && actual->num_OC != num_oc_filtro) {
            coincide = 0;
        }

        if (usar_filtro_estado && actual->estado != estado_filtro) {
            coincide = 0;
        }

        if (coincide) {
            imprimirProcesoOC(actual);
            encontrados++;
        }

        actual = actual->sig;
    }

    if (encontrados == 0) {
        printf("No se encontraron procesos que coincidan con el filtro.\n");
    }

    printf("------------------------------------------------------------\n");
}

/* * Guarda la lista de procesos OC en un archivo binario.

 * helper actualizado para version v1.0.1
 * @param nombre_archivo Nombre del archivo donde se guardarán los datos.
 * @param lista Puntero a la lista de procesos OC.
 * @param count Cantidad de procesos en la lista.
 * @return 0 en caso de éxito, código de error en caso contrario.
 */ 

int guardarProcesosOCEnDisco(const char *nombre_archivo, const ProcesoOC *lista, size_t count /* actualmente no usado */) {
    if (!nombre_archivo) {
        return OC_ERR_PARAM;
    }

    FILE *archivo = fopen(nombre_archivo, "wb");
    if (!archivo) {
        perror("Error al abrir archivo para escritura");
        return OC_ERR_IO;
    }

    oc_file_header header;
    header.magic = OC_FILE_MAGIC;   // 0x4F435052
    header.major = 1;
    header.minor = 1;

    if (fwrite(&header, sizeof(header), 1, archivo) != 1) {
        perror("Error al escribir el encabezado del archivo");
        fclose(archivo);
        return OC_ERR_IO;
    }

    // Recorremos la lista enlazada, no como arreglo
    const ProcesoOC *actual = lista;
    ProcesoOC_on_Disk_v1_0_1 temp;

    while (actual) {
        mem_to_disk_v1_0_1(actual, &temp);

        if (fwrite(&temp, sizeof(ProcesoOC_on_Disk_v1_0_1), 1, archivo) != 1) {
            perror("Error al escribir un registro en el archivo");
            fclose(archivo);
            return OC_ERR_IO;
        }

        actual = actual->sig;
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
    // intento de leer el encabezado
    uint32_t magic;
    size_t leidos = fread(&magic, sizeof(uint32_t), 1, archivo);
    if (leidos != 1) {
    // Puede ser EOF (archivo vacío) o error real
        if (feof(archivo)) {
            // Archivo vacío: lista vacía, caso normal
            fclose(archivo);
            *out_lista = NULL;
            *out_count = 0;
            return OC_OK;
        } else {
            // Error real de lectura
            perror("Error al leer encabezado de archivo");
            fclose(archivo);
            return OC_ERR_IO;
        }
    }
        // DEBUG
        printf("DEBUG leer: magic = 0x%08X\n", magic);
        fflush(stdout);
    if(magic == OC_FILE_MAGIC){
        //Nuevo formato v1.0.1
        oc_file_header header;
        header.magic = magic;
        if(fread(&header.major, sizeof(header.major), 1, archivo) != 1 ||
            fread(&header.minor, sizeof(header.minor), 1, archivo) != 1) {
            fclose(archivo);
            return OC_ERR_IO; // Error al leer el encabezado
        }
    
    
        // Calcular cantidad de registros
        if(fseek(archivo, 0, SEEK_END) != 0) {
            perror("Error al buscar el final del archivo");
            fclose(archivo);
            return OC_ERR_IO;
        }
    
        long tamanio_archivo = ftell(archivo);
            if(tamanio_archivo < 0) {
            perror("Error al obtener el tamaño del archivo");
            fclose(archivo);
            return OC_ERR_IO;
        }

        long offset_datos = (long)sizeof(oc_file_header);
            if(tamanio_archivo < offset_datos) {
            fclose(archivo);
            return OC_ERR_IO; // Archivo demasiado pequeño
        }

        long tamanio_datos = tamanio_archivo - offset_datos;
            if(tamanio_datos % (long)sizeof(ProcesoOC_on_Disk_v1_0_1) != 0) {
            fclose(archivo);
            return OC_ERR_IO; // Tamaño de datos inválido
        }
        size_t count = (size_t)(tamanio_datos / (long)sizeof(ProcesoOC_on_Disk_v1_0_1));

        if(fseek(archivo, offset_datos, SEEK_SET) != 0) {
            perror("Error al buscar el inicio de los datos en el archivo");
            fclose(archivo);
            return OC_ERR_IO;
        }

        ProcesoOC_on_Disk_v1_0_1 *buffer_disk = (ProcesoOC_on_Disk_v1_0_1 *)malloc(count * sizeof(ProcesoOC_on_Disk_v1_0_1));
        if (!buffer_disk) {
            perror("Error al asignar memoria para leer los procesos OC");
            fclose(archivo);
            return OC_ERR_MEMORIA;
        }

        if(fread(buffer_disk, sizeof(ProcesoOC_on_Disk_v1_0_1), count, archivo) != count) {
            perror("Error al leer los procesos OC desde el archivo");
            free(buffer_disk);
            fclose(archivo);
            return OC_ERR_IO;
        }

        fclose(archivo);

        


        // Convertir a lista enlazada en memoria (v1.0.1)
        ProcesoOC *head = NULL;
        ProcesoOC *tail = NULL;

        for (size_t i = 0; i < count; ++i) {
            ProcesoOC *nuevo = malloc(sizeof(ProcesoOC));
            if (!nuevo) {
                perror("Error al asignar memoria para la lista de procesos OC (v1.0.1)");

                // Liberar la lista ya construida
                ProcesoOC *tmp = head;
                while (tmp) {
                    ProcesoOC *next = tmp->sig;
                    free(tmp);
                    tmp = next;
                }

            free(buffer_disk);
            return OC_ERR_MEMORIA;
        }

        // Copiar datos desde el registro de disco
        disk_v1_0_1_to_mem(&buffer_disk[i], nuevo);
        nuevo->sig = NULL;  // MUY IMPORTANTE

        if (!head) {
            head = tail = nuevo;
        } else {
            tail->sig = nuevo;
            tail = nuevo;
        }
    }

        free(buffer_disk);

        *out_lista = head;
        *out_count = (uint32_t)count;
        return OC_OK;
    } else {
        // ===================== FORMATO LEGACY v1.0.0 =====================
        // Formato real:
        // [ uint32_t count ][ count * ProcesoOC_on_Disk ]

        // Los primeros 4 bytes (magic != OC_FILE_MAGIC) en realidad son 'count'
        uint32_t header_count = magic;

        // Si el count es 0, puede ser un archivo vacío con solo el header_count
        // Validamos el tamaño completo del archivo.
        if (fseek(archivo, 0, SEEK_END) != 0) {
            perror("Error al buscar el final del archivo legacy");
            fclose(archivo);
            return OC_ERR_IO;
        }

        long tamanio_archivo = ftell(archivo);
        if (tamanio_archivo < 0) {
            perror("Error al obtener el tamaño del archivo legacy");
            fclose(archivo);
            return OC_ERR_IO;
        }

        // Tamaño esperado: 4 bytes de count + N * sizeof(ProcesoOC_on_Disk)
        long esperado = 4 + (long)header_count * (long)sizeof(ProcesoOC_on_Disk);

        if (tamanio_archivo != esperado) {
            // El archivo no cuadra con el formato legacy esperado
            fprintf(stderr,
                "DEBUG IO: legacy size mismatch. tamanio_archivo=%ld, "
                "esperado=%ld, count=%u, sizeof(ProcesoOC_on_Disk)=%zu\n",
                tamanio_archivo, esperado, header_count,
                sizeof(ProcesoOC_on_Disk));
            fclose(archivo);
            return OC_ERR_IO;
        }

        // Si no hay registros (count == 0), devolvemos lista vacía
        if (header_count == 0) {
            fclose(archivo);
            *out_lista = NULL;
            *out_count = 0;
            return OC_OK;
    }

        // Posicionarnos justo donde empiezan los registros
        if (fseek(archivo, sizeof(uint32_t), SEEK_SET) != 0) {
            perror("Error al buscar el inicio de los datos legacy");
            fclose(archivo);
            return OC_ERR_IO;
        }

        // Reservar buffer para los registros en formato de disco
        size_t count = header_count;
        ProcesoOC_on_Disk *buffer_disk =
            malloc(count * sizeof(ProcesoOC_on_Disk));
        if (!buffer_disk) {
            perror("Error al asignar memoria para leer procesos legacy");
            fclose(archivo);
            return OC_ERR_MEMORIA;
        }

        if (fread(buffer_disk, sizeof(ProcesoOC_on_Disk), count, archivo) != count) {
            perror("Error al leer los procesos OC legacy desde el archivo");
            free(buffer_disk);
            fclose(archivo);
            return OC_ERR_IO;
        }

        fclose(archivo);

        // Convertir a lista enlazada en memoria (legacy v1.0.0)
        ProcesoOC *head = NULL;
        ProcesoOC *tail = NULL;

        for (size_t i = 0; i < count; ++i) {
            ProcesoOC *nuevo = malloc(sizeof(ProcesoOC));
            if (!nuevo) {
                perror("Error al asignar memoria para la lista de procesos OC (legacy)");

                // Liberar lista parcial ya construida
                ProcesoOC *tmp = head;
                while (tmp) {
                    ProcesoOC *next = tmp->sig;
                    free(tmp);
                    tmp = next;
                }

                free(buffer_disk);
                return OC_ERR_MEMORIA;
            }

            disk_v1_0_0_to_mem(&buffer_disk[i], nuevo);
            nuevo->sig = NULL;  // Esencial

            if (!head) {
                head = tail = nuevo;
            } else {
                tail->sig = nuevo;
                tail = nuevo;
            }   
        }   

        free(buffer_disk);

        *out_lista = head;
        *out_count = (uint32_t)count;
        return OC_OK;
    }
}
