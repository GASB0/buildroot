PROJECT_DIR := $(shell pwd)
PROM	= simple_filter_client

CFLAGS ?= -fPIC -O3 -I$(PROJECT_DIR) -lpthread -lasound
OBJ = main.o

$(PROM): $(OBJ)	
	$(CC) -o $(PROM) $(OBJ) $(CFLAGS)
	echo "$(CC)"
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
clean:
	@rm -rf $(OBJ) $(PROM)
