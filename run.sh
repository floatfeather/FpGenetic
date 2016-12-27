#!/bin/bash

make

cp ../test_program_generator/others/*.c others
cp ../test_program_generator/others/*.meta others

#cp pso/* others

cd others
files=`ls *.c`

for file in $files
do
	execfile=${file%.c}
	echo "Compile " $file " to " $execfile
	if [ -f $execfile ]; then
		rm $execfile
	fi
	g++ $file -L/usr/src/gsl-build/lib -lgsl1 -lgslcblas1 -L/usr/src/glibc-build/lib -lm1 -o $execfile
	if [ ! -f $execfile ]; then
		echo "Error when compiling " $execfile
	fi
done

cd ..

rm geneticlog/*
rm randomlog/*

for file in $files
do
	execfile=${file%.c}
	./main $execfile
done