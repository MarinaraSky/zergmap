#! /bin/bash
if ! [ -e ./zergmap ]
then
	make debug
fi
for x in $(ls mapper/pcaps); do
	echo $x 
	./zergmap mapper/pcaps/$x/*
	echo ================================
	done;
