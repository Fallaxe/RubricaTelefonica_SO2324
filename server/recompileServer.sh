#!/bin/bash
# -*- coding: utf-8 -*-
# compile server more quickly
gcc -c ./server.c;
gcc ./server.c -o server;
./server;
