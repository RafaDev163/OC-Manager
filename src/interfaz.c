#include <stdio.h>
#include <stdlib.h>
#include "procesosOC.h"
#include "interfaz.h"

void limpiar_pantalla(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pausar_pantalla(){
    printf("Presione Enter para continuar...");
    limpiar_buffer();
    getchar();
}
