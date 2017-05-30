#!/bin/bash

#while read filename
#do
#	cp $filename int
#done < ../test_program_generator/int_function.log

#cd int
#int_files=`ls *.c`
#cd ..

comp=$1

cd others/$comp
files=`ls *.c`
cd ../..

rm result_${comp}.csv
touch result_${comp}.csv
echo "function, genetic, random" >> result_${comp}.csv

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
	cd geneticlog/${comp}
	gen=0
	if [ -f $logfile ]; then
		while read line
		do
			gen=$line
		done < $logfile
	fi
	cd ../..
	cd randomlog/${comp}
	ran=0
	if [ -f $logfile ]; then
		while read line
		do
			ran=$line
		done < $logfile
	fi
	cd ../..
	echo $execfile $suffix "," $gen "," $ran >> result_${comp}.csv
done

