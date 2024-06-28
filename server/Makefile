OUT_DIR := ../build
BIN_DIR := ../bin
cJSON_DIR := ../vendor/cjson
LIBS := -lssl -lcrypto

$(BIN_DIR)/server: $(OUT_DIR)/server.o $(OUT_DIR)/server_utils.o $(OUT_DIR)/cJSON.o
	@mkdir -p $(@D)
	@gcc $(OUT_DIR)/server.o $(OUT_DIR)/cJSON.o $(OUT_DIR)/server_utils.o -o $(BIN_DIR)/server $(LIBS)
	@echo "Il file server è stato creato nella cartella $(BIN_DIR)"

$(OUT_DIR)/server.o: server.c server.h
	@mkdir -p $(@D)
	@gcc -c server.c -o $(OUT_DIR)/server.o

$(OUT_DIR)/server_utils.o: server_utils.c server_utils.h
	@mkdir -p $(@D)
	@gcc -c server_utils.c -o $(OUT_DIR)/server_utils.o
	
$(OUT_DIR)/cJSON.o: $(cJSON_DIR)/cJSON.c $(cJSON_DIR)/cJSON.h
	@mkdir -p $(@D)
	@gcc -c $(cJSON_DIR)/cJSON.c -o $(OUT_DIR)/cJSON.o
	
clean:
	@rm -rf $(OUT_DIR)
	@echo "Tutti i file *.o sono stati cancellati."

run:
	@echo "eseguendo il server da makefile..."
	@../bin/server