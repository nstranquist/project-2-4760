

all: runsim testsim

runsim: runsim.o license.o
	gcc -g -o runsim runsim.o license.o

testsim: testsim.o license.o
	gcc -g -o testsim testsim.o license.o

runsim.o: runsim.c config.h
	gcc -g -c runsim.c

testsim.o: testsim.c config.h
	gcc -g -c testsim.c

license.o: license.c license.h
	gcc -g -c license.c

clean:
	rm *.o runsim testsim