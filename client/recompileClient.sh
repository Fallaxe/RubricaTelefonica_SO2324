#!/bin/bash
# -*- coding: utf-8 -*-
# compile client more quickly
gcc -c ./client.c;
gcc ./client.c -o client;
./client;