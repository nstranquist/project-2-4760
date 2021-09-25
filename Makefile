

all: runsim testsim

runsim: runsim.c
	gcc -o runsim runsim.c

testsim: testsim.c
	gcc -o testsim testsim.c
