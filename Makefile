# Compilador
CC = gcc

# Directorios
SRCDIR = src
INCDIR = include
BINDIR = bin

# Flags de compilación
CFLAGS = -Wall -Wextra -std=c11 -g -I$(INCDIR)

# Archivos fuente
SRC = $(SRCDIR)/main.c \
      $(SRCDIR)/procesosOC.c \
      $(SRCDIR)/interfaz.c

# Objetos generados automáticamente
OBJ = $(SRC:.c=.o)

# Ejecutable final
TARGET = oc_manager

# Regla principal
all: $(TARGET)

# Como construir el ejecutable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

# Regla genérica para compilar cada archivo .c → .o
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpieza
clean:
	rm -f $(SRCDIR)/*.o $(TARGET)
