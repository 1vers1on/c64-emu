#!/bin/bash
ca65 6502test.s -o 6502bin/6502test.o
ld65 6502bin/6502test.o -o 6502test.bin -C cc65.cfg
