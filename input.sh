#!/bin/bash

make input_main

cp ../test_program_generator/others_pso/*.meta others

cd others
files=`ls *.meta`
cd ..

for file in $files
do
	execfile=${file%.c.meta}
	echo $execfile
	./input_main $execfile
done