run = mpirun
compiler = mpicc
target = kmeans.o
processes = 2
libs  = #-lkernel32 -luser32 -lgdi32 -lopengl32
cflags = #-Wall
filename = datasets/milione2.txt
num_clusters = 2

.PHONY : clean

$(target): KMeansClustering.c KMeansFileUtility.c
	$(compiler) -o $@ $^ $(cflags) $(libs)

run_local : $(target)
	$(run) -np $(processes) --oversubscribe -mca btl_base_warn_component_unused 0 ./$(target) $(filename) $(num_clusters)

run_distributed : $(target)
	$(run) -np $(processes) --oversubscribe --host node1,node2,node3 --mca btl_base_warn_component_unused 0 ./$(target) $(filename) $(num_clusters)

serial.o : SerialKMeansClustering.c
	   gcc -Wall SerialKMeansClustering.c -o serial.o

run_serial : serial.o
	     ./serial.o $(filename) $(num_clusters)

omp_kmeans.o : KMeansMPClustering.c
	   gcc -Wall KMeansMPClustering.c -o omp_kmeans.o -fopenmp

run_omp : omp_kmeans.o
		./omp_kmeans.o $(filename) $(num_clusters)
		rm omp_kmeans.o

clean :
	rm $(target) serial.o
