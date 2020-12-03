/*
* Parallel K-means clustering 
*/

#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
//#include <omp.h> TODO: uncomment
#include "KMeansClusteringDefs.h"

int main(int argc, char** argv){
    // Standard MPI code
    int communicator_size;
    int my_rank;

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &communicator_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // K-means
    int data_points_size;
    int attributes_size;
    int clusters_size; // how many clusters.. TODO: change "size" to more clear name
    int max_iterations; 
    int my_data_points_size;

    Cluster* clusters;
    ClusterDataPoint* my_data_points;

    if (my_rank == 0){
        // parse file
        char* filename;
        RawDataPoint* data_points;

        parseArgs(argc, argv, &filename, &clusters_size, &max_iterations);
        parseFile(filename, &data_points_size, &attributes_size, &data_points);

        // initialize clusters
        clusters = (Cluster*) malloc(sizeof(Cluster) * clusters_size);
        for (int i = 0; i < clusters_size; i++){
            clusters[i].cluster_id = i;
            clusters[i].centroid = data_points[i]; // Warning: This line of code probably doesnt work
        }
        // send data
    }
    else {
        // receive data
    }

    // do work


    MPI_Finalize();
    return 0; // Return correct status 
}

/**
function that parses the input arguments in:
-filename, the filename of the file where the data_points are defined
-clusters_size, the number of cluster to create
-max_iterations, if defined by the user is the maximum number of iterations of the alghoritm, otherwise is NULL

The function returns:
-1 if there was an error in parsing the arguments
0 otherwise.
**/ 
int parseArgs(int argc, char** argv, char** filename, int* clusters_size, int* max_iterations)
{
  if(argc<3)
    {
      printf("Usage of k-means: ./kmeans filename clusters_size [max_iterations]\n");
      return -1;
    }
  *filename = argv[1];
  *clusters_size = atoi(argv[2]);

  if(argc == 3) &max_iterations = -1;
  else
   {
     int max = atoi(argv[3]); 
     max_iterations = &max;
   }
  return 0;
}