#!/bin/bash

gcc $(ls *.c) --std=c99 -O3 -Wall && ./a.out
