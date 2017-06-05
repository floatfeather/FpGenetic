#!/bin/bash

# make

# cp ../test_program_generator/sf/*.c others/sf
# cp ../test_program_generator/sf/*.meta others/sf
# cp ../test_program_generator/stats/*.c others/stats
# cp ../test_program_generator/stats/*.meta others/stats

# #cp pso/* others

cd others/sf
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

cd ../..

rm geneticlog/sf/*
rm randomlog/sf/*
rm stdgeneticlog/sf/*

for file in $files
do
	execfile=${file%.c}
	echo $execfile
	if [ -e geneticlog/sf/${execfile}.log ]; then
		echo "already computed!"
	else
		./main $execfile sf > output.log 2>&1
	fi
done

cd others/stats
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

cd ../..

rm geneticlog/stats/*
rm randomlog/stats/*
rm stdgeneticlog/stats/*

for file in $files
do
	execfile=${file%.c}
	echo $execfile
	if [ -e geneticlog/stats/${execfile}.log ]; then
		echo "already computed!"
	else
		./main $execfile stats > output.log 2>&1
	fi
done