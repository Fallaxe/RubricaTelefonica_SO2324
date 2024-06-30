OUT_DIR := ../build
BIN_DIR := ../bin
cJSON_DIR := ../vendor/cjson
LIBS := -lssl -lcrypto

ARCH_DEPS := openssl
UBUNTU_DEPS := libssl-dev
FEDORA_DEPS := openssl-devel

INSTALLER_DEPS_DEBIAN = sudo apt-get update && sudo apt-get install
INSTALLER_DEPS_FEDORA = sudo dnf install
INSTALLER_DEPS_ARCH = sudo pacman -Syu

#ID_LIKE è una "variabile" definita nel file os-release che permette
#di individuare con successo il tipo di distribuzione in uso e così il package manager.
#grep -q return 1 se c'è il pattern.

define install_based_on_id_like
    if grep -q "ID_LIKE=.*debian.*" /etc/os-release; then \
        echo "Rilevata distribuzione Debian-like"; \
        $(INSTALLER_DEPS_DEBIAN) $(UBUNTU_DEPS); \
    elif grep -q "ID_LIKE=.*fedora.*" /etc/os-release; then \
        echo "Rilevata distribuzione Fedora-like"; \
        $(INSTALLER_DEPS_FEDORA) $(FEDORA_DEPS); \
    elif grep -q "ID_LIKE=.*arch.*" /etc/os-release; then \
        echo "Rilevata distribuzione Arch-like"; \
        $(INSTALLER_DEPS_ARCH) $(ARCH_DEPS); \
    else \
        echo "Distribuzione Linux non supportata"; \
        exit 1; \
    fi
endef

$(BIN_DIR)/server: $(OUT_DIR)/server.o $(OUT_DIR)/server_utils.o $(OUT_DIR)/cJSON.o
	@mkdir -p $(@D)
	@gcc $(OUT_DIR)/server.o $(OUT_DIR)/cJSON.o $(OUT_DIR)/server_utils.o -o $(BIN_DIR)/server $(LIBS)
	@echo "Il file server è stato creato nella cartella $(BIN_DIR)"
	@echo "Nel caso non compili usare 'make install' per installare una dipendenza esterna"
	@echo "Solitamente openssl in alcune disto di linux è preinstallato."

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

deps:
	@echo "installazione dipendenze."
	@$(install_based_on_id_like)