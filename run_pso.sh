#!/bin/bash

rm others/*

cp ../test_program_generator/others_pso/*.c others
cp ../test_program_generator/others_pso/*.meta others

cd ../test_program_generator/input_filter
ifiles=`ls *_f.txt`
for ifile in $ifiles
do
	raw=${ifile%_f.txt}
	mv $ifile ${raw}.txt
done
cd -

cp ../test_program_generator/input_filter/*_input.txt others

cd others
files=`ls *.c`

rm *pso*

for file in $files
do
	execfile=${file%.c}
	echo "Compile " $file " to " $execfile
	if [ -f $execfile ]; then
		rm $execfile
	fi
	g++ $file -L/usr/src/gsl-build/lib -lgsl1 -lgslcblas1 -L/usr/src/glibc-std-build/lib -lm2 -o $execfile
	if [ ! -f $execfile ]; then
		echo "Error when compiling " $execfile
		continue
	fi

	valgrind --tool=fpdebug --detect-pso=yes ./$execfile > temp.log 2>&1
	rm temp.log
	rm $execfile
done

cd ..