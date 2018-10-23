#! /bin/bash
for x in $(ls mapper/pcaps); do
	echo $x 
	./decode mapper/pcaps/$x/*
	echo ================================
	done;
