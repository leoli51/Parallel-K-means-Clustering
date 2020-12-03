/* This header contains the definitions of the datatypes needed by the k-means algorithm  */
#ifndef K_MEANS_DATATYPES
#define K_MEANS_DATATYPES

#define MAX_FILENAME_SIZE 128

typedef struct {
    float* attributes;
} RawDataPoint;

typedef struct {
    int cluster_id;
    RawDataPoint data_point; // TODO: should it be a pointer or the actual struct object?
} ClusterDataPoint;

typedef struct {
    int cluster_id;
    RawDataPoint centroid;
} Cluster;


// parsing prototypes
int parseArgs(int argc, char** argv, char* filename, int* clusters_size, int* max_iterations);
int parseFile(const char* filename,int* data_points_size, int* attributes_size, RawDataPoint** array_of_datapoints);

// k-means prototype
int assignPointsToNearestCluster(...);
int updateClusters(...);
...

#endif /* !K_MEANS_DATATYPES */

/**
function that parses the input arguments in:
-filename, the filename of the file where the data_points are defined
-clusters_size, the number of cluster to create
-max_iterations, if defined by the user is the maximum number of iterations of the alghoritm, otherwise is NULL

The function returns:
-1 if there was an error in parsing the arguments
0 otherwise.
**/ 
int parseArgs(int argc, char** argv, char* filename, int* clusters_size, int* max_iterations)
{
  if(argc<3)
    {
      printf("Usage of k-means: ./kmeans filename clusters_size [max_iterations]\n");
      return -1;
    }
  filename = argv[1];
  int k = atoi(argv[2]);
  clusters_size = &k;
  if(argc == 3) max_iterations = NULL;
  else
   {
     int max = atoi(argv[3]); 
     max_iterations = &max;
   }
  return 0;
}
