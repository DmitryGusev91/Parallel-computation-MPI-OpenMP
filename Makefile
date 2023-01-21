build:
	mpicxx -fopenmp -c main.c -o main.o
	mpicxx -fopenmp -c cFunc.c -o cFunc.o
	mpicxx -fopenmp -o program main.o cFunc.o 
	
clean:
	rm -f *.o ./program

run:
	mpiexec -np 2 ./program
	
runOn2:
	mpiexec -np 2 -hostfile myHost.txt ./program
	

		
		
		
