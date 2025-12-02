#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "procesosOC.h"
#include "interfaz.h"


int main(){
    printf("Programa de gestión de procesos de órdenes de compra.\n");
    // Aquí iría la lógica principal del programa
    ProcesoOC *lista_procesos = NULL;
    int opc;

    uint32_t count = 0; // cuenta para la lista de procesos OC
    // Cargar procesos OC desde disco al iniciar el programa
    int lectura = leerProcesosOCDesdeDisco("procesos_oc.bin", &lista_procesos, &count); // Leer hasta 100 procesos

    if (lectura != OC_OK) {
    printf("Error al leer procesos OC desde disco. Código de error: %d\n", lectura);
    return 1;
    }

    if (count == 0) {
    printf("No hay procesos OC previos. Iniciando lista vacía.\n");
    } else {
    printf("Se cargaron %u procesos OC desde disco.\n", count);
    }
    
    /* Debug: listar procesos cargados 
    Bug solucionado en la funcion leerProcesosOCDesdeDisco, 
    linea 236 src/procesosOC.c
    printf("[DEBUG] Lista cargada al inicio:\n");
    listarProcesosOC(lista_procesos);
    */

    

    //menu principal
    do {
        limpiar_pantalla(); // Limpiar pantalla al inicio de cada iteración

        printf("\nMenú Principal:\n");
        printf("1. Agregar Proceso OC\n");
        printf("2. Iniciar Etiquetado OC\n");
        printf("3. Finalizar Etiquetado OC\n");
        printf("4. Listar Procesos OC\n");
        printf("5. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opc);
        limpiar_buffer(); // Limpiar buffer después de leer la opción para evitar basura en stdin
        

        switch(opc) {
            case 1:
                limpiar_pantalla();
                // Lógica para agregar proceso OC
                int num_OC;
                char nombre_prov[MAX_SIZE_NOMBRE_PROVEEDOR];
                char nombre_etiq[MAX_SIZE_NOMBRE_ETIQUETADOR];
                size_t cant_productos;
                // Solicita los datos al usuario
            
                int scan_num_oc;
                do {
                    printf("Ingrese un número de OC: ");
                    scan_num_oc = scanf("%d", &num_OC);
                    if (scan_num_oc != 1) {
                        printf("Entrada inválida. Por favor ingrese un número.\n");
                        limpiar_buffer();
                    }
                    if(scan_num_oc == 1 && num_OC < 0) {
                        printf("El número de OC no puede ser negativo.\n");
                        scan_num_oc = 0; // Forzar repetición
                    }
                } while (scan_num_oc != 1 || num_OC < 0);

                
                int scan_prod;
                do {
                    printf("Ingrese cantidad de productos: ");
                    scan_prod = scanf("%zu", &cant_productos);
                    if (scan_prod != 1) {
                        printf("Entrada inválida. Por favor ingrese un número.\n");
                        limpiar_buffer();
                    }
                    if(scan_prod == 1 && cant_productos == 0) {
                        printf("La cantidad de productos debe ser mayor que cero.\n");
                        scan_prod = 0; // Forzar repetición
                    }

                } while (scan_prod != 1);

                limpiar_buffer();
                printf("Ingrese nombre del proveedor: ");
                fgets(nombre_prov, MAX_SIZE_NOMBRE_PROVEEDOR, stdin);
                nombre_prov[strcspn(nombre_prov, "\n")] = 0; // Eliminar salto de línea
                printf("Ingrese nombre del etiquetador: ");
                fgets(nombre_etiq, MAX_SIZE_NOMBRE_ETIQUETADOR, stdin);
                nombre_etiq[strcspn(nombre_etiq, "\n")] = 0; // Eliminar salto de línea
               
                int resultado = agregarProcesoOC(&lista_procesos, num_OC, nombre_prov, nombre_etiq, cant_productos);
                if (resultado == OC_OK) {
                    printf("Proceso OC agregado exitosamente.\n");
                    guardarProcesosOCEnDisco("procesos_oc.bin", lista_procesos);
                } else if (resultado == OC_ERR_DUPLICADA) {
                    printf("Error: Ya existe una OC con ese número.\n");
                } else {
                    printf("Error al agregar proceso OC. Código de error: %d\n", resultado);
                }
                pausar_pantalla();
                break;
            case 2:
                limpiar_pantalla();
                //Logica para iniciar etiquetado OC
                // Solicita el número de OC, valida la entrada, busca el proceso y llama a iniciarEtiquetadoOC
                printf("Ingrese número de OC a iniciar etiquetado: ");
                int oc_iniciar;
                int scan_result;
                do {
                    printf("Ingrese un número válido de OC: ");
                    scan_result = scanf("%d", &oc_iniciar);
                    if (scan_result != 1) {
                        printf("Entrada inválida. Por favor ingrese un número.\n");
                        limpiar_buffer();
                    }
                    if(scan_result == 1 && oc_iniciar < 0) {
                        printf("El número de OC no puede ser negativo.\n");
                        scan_result = 0; // Forzar repetición
                    }
                } while (scan_result != 1 || oc_iniciar < 0);
                ProcesoOC *proceso_iniciar = buscarProcesoOC(lista_procesos, oc_iniciar);
                if (proceso_iniciar) {
                    int res_iniciar = iniciarEtiquetadoOC(proceso_iniciar);
                    if (res_iniciar == OC_OK) {
                        printf("Etiquetado iniciado para OC Nro: %d\n", oc_iniciar);
                        guardarProcesosOCEnDisco("procesos_oc.bin", lista_procesos);
                    } else {
                        printf("Error al iniciar etiquetado. Código de error: %d\n", res_iniciar);
                    }
                } else {
                    printf("OC Nro: %d no encontrada.\n", oc_iniciar);
                }
                pausar_pantalla();
                break;
            case 3:
                limpiar_pantalla();
                // Lógica para finalizar etiquetado OC
                printf("Ingrese número de OC a finalizar etiquetado: ");
                int oc_finalizar;
                int scan_finalizar;
                do {
                    printf("Ingrese un número válido de OC: ");
                    scan_finalizar = scanf("%d", &oc_finalizar);
                    if (scan_finalizar != 1) {
                        printf("Entrada inválida. Por favor ingrese un número.\n");
                        limpiar_buffer();
                    }
                    if(scan_finalizar == 1 && oc_finalizar < 0) {
                        printf("El número de OC no puede ser negativo.\n");
                        scan_finalizar = 0; // Forzar repetición
                    }
                } while (scan_finalizar != 1 || oc_finalizar < 0);
                ProcesoOC *proceso_finalizar = buscarProcesoOC(lista_procesos, oc_finalizar);
                if (proceso_finalizar) {
                    int res_finalizar = finalizarEtiquetadoOC(proceso_finalizar);
                    if (res_finalizar == OC_OK) {
                        printf("Etiquetado finalizado para OC Nro: %d\n", oc_finalizar);
                        guardarProcesosOCEnDisco("procesos_oc.bin", lista_procesos);
                    } else if (res_finalizar == OC_ERR_NO_INICIADO) {
                        printf("Error: El etiquetado no ha sido iniciado para OC Nro: %d\n", oc_finalizar);
                    } else if (res_finalizar == OC_ERR_YA_FINALIZADO) {
                        printf("Error: El etiquetado ya fue finalizado para OC Nro: %d\n", oc_finalizar);
                    } else {
                        printf("Error al finalizar etiquetado. Código de error: %d\n", res_finalizar);
                    }
                } else {
                    printf("OC Nro: %d no encontrada.\n", oc_finalizar);
                }
                pausar_pantalla();
                break;
            case 4:
                limpiar_pantalla();
                listarProcesosOC(lista_procesos);
                pausar_pantalla();
                break;
            case 5:
                guardarProcesosOCEnDisco("procesos_oc.bin", lista_procesos);
                printf("Saliendo del programa.\n");
                break;
            default:
                printf("Opción inválida. Intente de nuevo.\n");
        }
    } while(opc != 5);

    liberarListaProcesosOC(&lista_procesos);
    
    return 0;
}