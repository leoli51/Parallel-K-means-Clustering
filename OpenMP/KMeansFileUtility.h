#ifndef K_MEANS_FILE_UTILITY
#define K_MEANS_FILE_UTILITY

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "KMeansClusteringDefs.h"

// parsing prototypes
int parseArgs(int argc, char** argv, char** filename, int* clusters_size, int* max_iterations);
int parseFile(const char* filename,int* data_points_size, int* attributes_size, ClusterDataPoint** array_of_datapoints);
int printResult(char *filename,ClusterDataPoint* clusters, int num_clusters, int num_attributes);
int readLine(FILE* file,char* line);

#endif
