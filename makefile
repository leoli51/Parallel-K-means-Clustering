processes = 4
dataset = datasets/dieci2.txt
num_clusters = 2

.PHONY : clean

mpi_kmeans.o : KMeansClustering.c KMeansFileUtility.c
	mpicc -o $@ $^

run_mpi : mpi_kmeans.o
	mpirun -np $(processes) --oversubscribe -mca btl_base_warn_component_unused 0 ./mpi_kmeans.o $(dataset) $(num_clusters)

#run_distributed : $(target)
#	mpirun -np $(processes) --oversubscribe --host node1,node2,node3 --mca btl_base_warn_component_unused 0 ./$mpi_kmeans.o $(dataset) $(num_clusters)

serial_kmeans.o : SerialKMeansClustering.c
	   gcc -Wall SerialKMeansClustering.c -o serial_kmeans.o

run_serial : serial_kmeans.o
	     ./serial_kmeans.o $(dataset) $(num_clusters)

omp_kmeans.o : KMeansMPClustering.c
	   gcc -Wall KMeansMPClustering.c -o omp_kmeans.o -fopenmp

run_omp : omp_kmeans.o
		./omp_kmeans.o $(dataset) $(num_clusters)

pthread_kmeans.o : PThreadKmeansClustering.c KMeansFileUtility.c
		gcc -Wall PThreadKmeansClustering.c KMeansFileUtility.c -o pthread_kmeans.o -pthread

run_pthread : pthread_kmeans.o
		./pthread_kmeans.o $(processes) $(dataset) $(num_clusters)

clean :
	rm *.o

clean_results :
	rm *.txt
