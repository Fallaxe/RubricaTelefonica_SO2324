#!/bin/bash
# -*- coding: utf-8 -*-
# compile server more quickly
mkdir -p ../build;
gcc -c ./server.c -o ../build/server.o;
mkdir -p ../bin;

# "-l" per linkare la libreria cjson 
# (intallarla con il pakage manager, successivamente cartella vendor con .h e .c della libreria?)
gcc ../build/server.o -o ../bin/server -l cjson;
../bin/server ;
