/*
* Parallel K-means clustering
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"
//#include <omp.h> TODO: uncomment
#include "KMeansClusteringDefs.h"
#include "KMeansFileUtility.h"
#include "KMeansMPIUtils.h"

#include <unistd.h>

int main(int argc, char** argv){
    // Standard MPI code
    int communicator_size;
    int my_rank;

    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, &communicator_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // K-means
    int num_data_points;
    int num_attributes;
    int num_clusters;
    int max_iterations;


    Cluster* clusters;
    ClusterDataPoint* my_data_points;
    RawDataPoint* raw_data_points;
    RawDataPoint* my_raw_data_points;
    int num_my_data_points;

    if (my_rank == 0){
        // parse file
        char* filename;
        if(parseArgs(argc, argv, &filename, &num_clusters, &max_iterations) == -1) return -1;
        if(num_clusters <= 1) { printf("Too few clusters, you should use a minimum of 2 clusters\n"); return -1; }
        if(parseFile(filename, &num_data_points, &num_attributes, &raw_data_points) == -1) return -1;
        if(num_clusters >= num_data_points) { printf("Too few datapoints, they should be more than the clusters\n"); return -1; } //TODO inserire il controllo in parseFile è meglio
        if(num

    }

    #if DEBUG
    if (my_rank == 0)
        printPoints(raw_data_points, num_data_points, num_attributes);
    #endif

    // send/receive information to create datatypes
    int information_buffer[4] = {num_data_points, num_attributes, num_clusters, max_iterations};
    MPI_Bcast(information_buffer, 4, MPI_INT, 0, MPI_COMM_WORLD);

    #if DEBUG
    if (my_rank == 0){
        printf("%d %d\n", num_data_points, num_attributes);
        printPoints(raw_data_points, num_data_points, num_attributes);
    }
    #endif

    // copy values from buffer to vars
    if (my_rank != 0){
        num_data_points = information_buffer[0];
        num_attributes = information_buffer[1];
        num_clusters = information_buffer[2];
        max_iterations = information_buffer[3];
    }

    // initialize clusters
    clusters = (Cluster*) malloc(sizeof(Cluster) * num_clusters);
    for (int i = 0; i < num_clusters; i++){
        clusters[i].cluster_id = i;
    }


    // initialize cluster buffer
    float** cluster_buffer = (float**) malloc(num_clusters*sizeof(float*));
    for (int i = 0; i < num_clusters; i++){
        cluster_buffer[i] = (float*) malloc(sizeof(float) * num_attributes);
        if (my_rank == 0){
            for (int j = 0; j < num_attributes; j++){
                cluster_buffer[i][j] = raw_data_points[i].attributes[j];
            }
        }
      }

    // create MPI datatype
    MPI_Datatype mpi_cluster_buffer_type;
    buildMPIPointBufferType(&mpi_cluster_buffer_type, num_attributes, cluster_buffer[1] - cluster_buffer[0], num_clusters);
    MPI_Type_commit(&mpi_cluster_buffer_type);

    #if DEBUG
    if (my_rank == 0)
        printPoints(raw_data_points, num_data_points, num_attributes);
    #endif

    MPI_Bcast(*cluster_buffer, 1, mpi_cluster_buffer_type, 0, MPI_COMM_WORLD);

    #if DEBUG
    if (my_rank == 0)
        printPoints(raw_data_points, num_data_points, num_attributes);
    #endif

    // assign buffer attributes to cluster struct
    for (int i = 0; i < num_clusters; i++){
        clusters[i].centroid.attributes = cluster_buffer[i];
    }

    free(cluster_buffer);
    MPI_Type_free(&mpi_cluster_buffer_type);

    #if DEBUG
    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < num_clusters; i++){
      printf("Rank: %d\tCluster %d: ", my_rank, clusters[i].cluster_id);
      for (int j = 0; j < num_attributes; j++)
        printf(" %f", clusters[i].centroid.attributes[j]);
      printf("\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    #endif

    /*// send data points
    int num_data_points_per_worker = num_data_points / (communicator_size);
    int num_data_points_extra_per_master = num_data_points % (communicator_size);

    num_my_data_points = num_data_points_per_worker;
    if (my_rank == 0)
        num_my_data_points += num_data_points_extra_per_master;

    float** data_points_send_buffer = NULL;
    if (my_rank == 0){
        int send_buffer_length = num_data_points_per_worker * communicator_size;
        data_points_send_buffer = (float**) malloc(sizeof(float*) * send_buffer_length);
        for (int i = 0; i < send_buffer_length; i++)
            data_points_send_buffer[i] = raw_data_points[i].attributes;
    }

    my_raw_data_points = (RawDataPoint*) malloc(sizeof(RawDataPoint) * num_my_data_points);
    float** data_points_receive_buffer = (float**) malloc(sizeof(float*) * num_my_data_points);
    for (int i = 0; i < num_my_data_points; i++){
        my_raw_data_points[i].attributes = (float*) malloc(sizeof(float) * num_attributes);
        data_points_receive_buffer[i] = my_raw_data_points[i].attributes;
    }

    MPI_Datatype mpi_point_buffer_type;
    buildMPIPointBufferType(&mpi_point_buffer_type, num_attributes, data_points_receive_buffer[1] - data_points_receive_buffer[0], num_poin);
    MPI_Type_commit(&mpi_point_buffer_type);

    printf("Yes i got here %d\n", my_rank);

    MPI_Scatter(*data_points_send_buffer, 1, mpi_point_buffer_type,
                *data_points_receive_buffer, 1, mpi_point_buffer_type, 0, MPI_COMM_WORLD);*/

    // how many raw data points should each process receive
    int num_data_points_per_worker = num_data_points / (communicator_size);
    int remaining = num_data_points % communicator_size;

    int* send_receive_count_buffer = (int*) malloc(sizeof(int) * communicator_size);
    send_receive_count_buffer[0] = num_attributes * num_data_points_per_worker;
    for (int i = 1; i < communicator_size; i++){
      send_receive_count_buffer[i] = num_data_points_per_worker;
      if (remaining > 0){
          send_receive_count_buffer[i]++;
          remaining--;
      }
      send_receive_count_buffer[i] *= num_attributes;
    }

    #if DEBUG
    printf("R: %d, Should Receive: %d attributes\n", my_rank, send_receive_count_buffer[my_rank]);
    if (my_rank == 0)
        printf("Total attributes: %d\n", num_data_points * num_attributes);
    #endif

    // data points send buffer
    float* data_points_send_buffer = NULL;
    if (my_rank == 0){
        data_points_send_buffer = (float*) malloc(sizeof(float) * num_data_points * num_attributes);
        printf("\n\nSend buffer: %p\n\n", data_points_send_buffer);
        for (int i = 0; i < num_data_points; i++){
            printf("\ndata point %d\n\t", i);
            for (int j = 0; j < num_attributes; j++){
                printf("%f ", raw_data_points[i].attributes[j]);
                data_points_send_buffer[i*num_attributes + j] = raw_data_points[i].attributes[j];
            }
        }
        printf("\n\nSend buffer: %p\n\n", data_points_send_buffer);

    }

    // data points receive buffer
    float* data_points_receive_buffer = (float*) malloc(sizeof(float) * num_attributes * send_receive_count_buffer[my_rank]);

    // set displacements of data points for each process
    int* displacements_buffer = NULL;
    if (my_rank == 0){
        displacements_buffer = (int*) malloc(sizeof(int) * communicator_size);
        displacements_buffer[0] = 0;              // offsets into the global array
        for (int i = 1; i < communicator_size; i++)
            displacements_buffer[i] = displacements_buffer[i-1] + send_receive_count_buffer[i-1];
    }

    MPI_Scatterv(data_points_send_buffer, send_receive_count_buffer, displacements_buffer, MPI_FLOAT,
                 data_points_receive_buffer, send_receive_count_buffer[my_rank], MPI_FLOAT, 0, MPI_COMM_WORLD);

    #if DEBUG
    MPI_Barrier(MPI_COMM_WORLD);

    printf("Rank: %d, Received: %d Points", my_rank, num_my_data_points);

    MPI_Barrier(MPI_COMM_WORLD);
    #endif

    free(data_points_receive_buffer);
    free(data_points_send_buffer);
    //free(send_receive_count_buffer);
    //free(displacements_buffer);

    // do work


    // free cluster memory
    for (int i = 0; i < num_clusters; i++){
      free(clusters[i].centroid.attributes);
    }
    free(clusters);

    // free data points.


    MPI_Finalize();
    return 0; // Return correct status
}

int assignPointsToNearestCluster(ClusterDataPoint* my_raw_data,Cluster* clusters,int num_attributes,int my_raw_data_num,int num_clusters)
{
  int i,j,n_attr,cluster_index;
  float distance,min_distance,temp;
  ClusterDataPoint point;
  Cluster cluster;
  #pragma omp parallel for
  for(i = 0; i < my_raw_data_num; i++) //for every point to analyze
   {
     point = my_raw_data[i]; //take a point
     for(j = 0; j < num_clusters; j++) //for every cluster
      {
        distance = 0;
        cluster = clusters[j]; //take a cluster
 	for(n_attr = 0; n_attr < num_attributes; n_attr++) //calculate the distance between the point and the cluster
 	 {
 	   temp = (point.data_point.attributes[n_attr] - cluster.centroid.attributes[n_attr]);
 	   distance += temp * temp;
 	 }
 	if(j == 0 || distance < min_distance) //if it is the first cluster or it is the nearest up to now
 	{
 	  min_distance = distance; //consider it as the nearest
 	  cluster_index = j;
 	}
      }
     point.cluster_id = cluster_index; //assign the nearest cluster to the point
   }
  return 0;
}
