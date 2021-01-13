compiler = gcc
target = omp_kmeans.o
src = $(wildcard *.c)
cflags = -Wall
libs = -fopenmp

.PHONY : clean

$(target): $(src)
	$(compiler) -D NUM_ATTRIBUTES=$(attributes) -o $@ $^ $(cflags) $(libs)

run_omp : $(target)
		./$(target) $(dataset) $(clusters)
		rm $(target)

clean :
	rm $(target)
