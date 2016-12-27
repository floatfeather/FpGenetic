objects = main.o status.o argument.o genetic.o
objects_input = input_main.o status.o argument.o genetic.o
CXXFLAGS=-std=c++0x

main : $(objects)
	g++ -o main $(objects)

input_main: $(objects_input)
	g++ -o input_main $(objects_input)

main.o: status.h genetic.h
input_main.o: status.h genetic.h
status.o: status.h
argument.o: argument.h
genetic.o: genetic.h status.h argument.h

.PHONY : clean
	-rm main $(object)