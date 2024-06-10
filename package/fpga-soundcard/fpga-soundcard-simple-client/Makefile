PROJECT_DIR := $(shell pwd)
PROM	= simple_filter_client

CFLAGS ?= -fPIC -O0 -I$(PROJECT_DIR) -lpthread -lasound -g3
OBJ = main.o

$(PROM): $(OBJ)	
	$(CC) -o $(PROM) $(OBJ) $(CFLAGS)
	echo "$(CC)"
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
clean:
	@rm -rf $(OBJ) $(PROM)
