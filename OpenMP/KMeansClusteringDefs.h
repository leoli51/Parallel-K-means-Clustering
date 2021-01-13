/* This header contains the definitions of the datatypes needed by the k-means algorithm  */
#ifndef K_MEANS_DATATYPES
#define K_MEANS_DATATYPES

#define MAX_INTEGER_LENGTH 20
#define MAX_FLOAT_LENGTH 64
#define DEBUG 1

typedef struct {
    int cluster_id;
    float attributes[NUM_ATTRIBUTES];
} ClusterDataPoint __attribute__ ((aligned (16)));

#endif /* !K_MEANS_DATATYPES */
