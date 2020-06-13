#!/bin/sh

make -f Makefile.wii uqm-wii
elf2dol uqm-wii uqm-wii.dol
cp uqm-wii.dol boot.dol

