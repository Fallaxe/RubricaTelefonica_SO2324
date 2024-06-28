#!/bin/bash
# -*- coding: utf-8 -*-
# compile server more quickly
mkdir -p ../build;
gcc -c ../vendor/cjson/cJSON.c -o ../build/cJSON.o;
gcc -c ../vendor/cjson/cJSON_Utils.c -o ../build/cJSON_Utils.o;
gcc -c ./server_utils.c -o ../build/server_utils.o;
gcc -c ./server.c -o ../build/server.o;
mkdir -p ../bin;

# "-l" per linkare la libreria cjson 
# (intallarla con il pakage manager, successivamente cartella vendor con .h e .c della libreria?)
gcc ../build/server.o ../build/cJSON.o ../build/cJSON_Utils.o ../build/server_utils.o -o ../bin/server -lssl -lcrypto;
../bin/server ;
