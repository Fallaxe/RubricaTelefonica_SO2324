OUT_DIR := ../build
BIN_DIR := ../bin

$(BIN_DIR)/client: $(OUT_DIR)/client.o
	@mkdir -p $(@D)
	@gcc $(OUT_DIR)/client.o -o $(BIN_DIR)/client
	@echo "Il file client è stato creato nella cartella $(BIN_DIR)"

$(OUT_DIR)/client.o: client.c client.h
	@mkdir -p $(@D)
	@gcc -c client.c -o $(OUT_DIR)/client.o
	
clean:
	@rm -rf $(OUT_DIR)
	@echo "Tutti i file *.o sono stati cancellati."
run:
	@echo "eseguendo un client da makefile..."
	@../bin/client