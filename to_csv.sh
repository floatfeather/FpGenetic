#!/bin/bash

while read filename
do
	cp $filename int
done < ../test_program_generator/int_function.log

cd int
int_files=`ls *.c`
cd ..

cd others
files=`ls *.c`
cd ..

rm result.csv
touch result.csv
echo "function, genetic, random" >> result.csv

for file in $files
do
	suffix=
	for int_file in $int_files
	do
		if [ "$int_file" = "$file" ]; then
			suffix="#"
			echo $suffix
		fi
	done
	execfile=${file%.c}
	logfile=${execfile}.log
	cd geneticlog
	gen=
	if [ -f $logfile ]; then
		while read line
		do
			gen=$line
		done < $logfile
	fi
	cd ..
	cd randomlog
	ran=
	if [ -f $logfile ]; then
		while read line
		do
			ran=$line
		done < $logfile
	fi
	cd ..
	echo $execfile $suffix "," $gen "," $ran >> result.csv
done

