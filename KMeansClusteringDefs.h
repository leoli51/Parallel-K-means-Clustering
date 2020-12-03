/* This header contains the definitions of the datatypes needed by the k-means algorithm  */
#ifndef K_MEANS_DATATYPES
#define K_MEANS_DATATYPES

#define DEBUG 1

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
int parseArgs(int argc, char** argv, char** filename, int* clusters_size, int* max_iterations);
int parseFile(const char* filename,int* data_points_size, int* attributes_size, RawDataPoint** array_of_datapoints);

// k-means prototype
int assignPointsToNearestCluster(...);
int updateClusters(...);
...

#endif /* !K_MEANS_DATATYPES */