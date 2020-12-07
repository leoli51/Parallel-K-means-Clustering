/* This header contains the definitions of the datatypes needed by the k-means algorithm  */
#ifndef K_MEANS_DATATYPES
#define K_MEANS_DATATYPES

#define MAX_INTEGER_LENGTH 10
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


// k-means prototype
int assignPointsToNearestCluster(ClusterDataPoint* my_raw_data,Cluster* clusters,int attributes_size,int my_raw_data_num,int num_clusters);
//int updateClusters(...);


#endif /* !K_MEANS_DATATYPES */
