/* This header contains the definitions of the datatypes needed by the k-means algorithm  */
#ifndef K_MEANS_DATATYPES
#define K_MEANS_DATATYPES

#define MAX_INTEGER_LENGTH 20
#define MAX_FLOAT_LENGTH 64
#define DEBUG 1

typedef struct {
    float* attributes;
} RawDataPoint;

typedef struct {
    int cluster_id;
    RawDataPoint data_point;
} ClusterDataPoint;

typedef struct {
    int cluster_id;
    RawDataPoint centroid;
} Cluster;

// communication
int sendPoints( int my_rank, int communicator_size, int num_data_points, int num_attributes,
                int* num_my_data_points,
                RawDataPoint* raw_data_points, RawDataPoint** my_raw_data_points, ClusterDataPoint** my_data_points);

int sendClusters(int my_rank, int num_clusters, int num_attributes, Cluster* clusters, RawDataPoint* raw_data_points);
int synchronizeClusters(int my_rank, int num_clusters, int num_attributes, Cluster* clusters, int* points_per_cluster);

int gatherDataPoint(int my_rank, ClusterDataPoint* my_points, int num_my_points, int comm_size, int num_data_points, int* clustered_points);


// k-means prototype
int assignPointsToNearestCluster(ClusterDataPoint* my_raw_data,Cluster* clusters,int num_attributes,int my_raw_data_num,int num_clusters,_Bool* hasChanged,int* num_points_per_cluster);
int updateLocalClusters(int my_rank, int num_attributes, int num_my_data_points, ClusterDataPoint* my_data_points, int num_clusters, Cluster* clusters);



#endif /* !K_MEANS_DATATYPES */
