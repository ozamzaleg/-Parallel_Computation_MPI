build:
	mpicc -o static static.c function.c -lm
	mpicc -o dynamic dynamic.c function.c -lm

clean:
	rm -rf *.o static dynamic
