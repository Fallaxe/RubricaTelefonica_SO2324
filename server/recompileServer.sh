#!/bin/bash
# -*- coding: utf-8 -*-
# compile server more quickly
mkdir -p ../build;
gcc -c ./server.c -o ../build/server.o;
mkdir -p ../bin;
gcc ../build/server.o -o ../bin/server;
../bin/server;
