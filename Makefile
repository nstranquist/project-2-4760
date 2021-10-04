

all: runsim testsim

runsim: runsim.o license.o
	gcc -g -o runsim runsim.o license.o

testsim: testsim.o license.o
	gcc -g -o testsim testsim.o license.o

runsim.o: runsim.c
	gcc -g -c runsim.c

testsim.o: testsim.c
	gcc -g -c testsim.c

license.o: license.c
	gcc -g -c license.c

clean:
	rm *.o runsim testsim