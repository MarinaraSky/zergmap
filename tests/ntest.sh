#! /bin/bash
for x in $(ls mapper/pcaps); do
	echo $x 
	./zergmap mapper/pcaps/$x/* -h 85
	echo ================================
	done;
