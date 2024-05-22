#!/bin/bash
# -*- coding: utf-8 -*-
# compile client more quickly
mkdir -p ../build;
gcc -c ./client.c -o ../build/client.o;
mkdir -p ../bin;
gcc ../build/client.o -o ../bin/client;
../bin/client;