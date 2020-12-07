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

        parseArgs(argc, argv, &filename, &num_clusters, &max_iterations);
        parseFile(filename, &num_data_points, &num_attributes, &raw_data_points);
    }

    // send/receive information to create datatypes
    int information_buffer[4] = {num_data_points, num_attributes, num_clusters, max_iterations};
    MPI_Bcast(information_buffer, 4, MPI_INT, 0, MPI_COMM_WORLD);

    // copy values from buffer to vars
    if (my_rank != 0){
        num_data_points = information_buffer[0];
        num_attributes = information_buffer[1];
        num_clusters = information_buffer[2];
        max_iterations = information_buffer[3];
    }
    
    // create MPI datatype
    MPI_Datatype mpi_raw_point_type;
    buildMPIRawDataPointType(&mpi_raw_point_type, num_attributes);
    MPI_Type_commit(&mpi_raw_point_type);


    // initialize clusters
    clusters = (Cluster*) malloc(sizeof(Cluster) * num_clusters);
    for (int i = 0; i < num_clusters; i++){
        clusters[i].cluster_id = i;
    }

    // initialize cluster buffer
    float** cluster_buffer = (float**) malloc(num_clusters*sizeof(float*));
    for (int i = 0; i < num_clusters; i++){
        cluster_buffer[i] = (float*) malloc(sizeof(float) * num_attributes);
        if (my_rank == 0)
          memcpy(cluster_buffer[i], raw_data_points[i].attributes, sizeof(float) * num_attributes);
    }

    // send/receive cluster information
    MPI_Bcast(cluster_buffer, num_clusters, mpi_raw_point_type, 0, MPI_COMM_WORLD);

    // assign buffer attributes to cluster struct
    for (int i = 0; i < num_clusters; i++){
        clusters[i].centroid.attributes = cluster_buffer[i];
    }

    #if DEBUG
    printf("Rank:%d, Clusters:\n",my_rank);
    for (int i = 0; i < num_clusters; i++){
      printf("Cluster %d: ", clusters[i].cluster_id);
      for (int j = 0; j < num_attributes; j++)
        printf("%f", clusters[i].centroid.attributes[j]);
      printf("\n");
    }
    #endif

    // send data points
    int num_data_points_per_worker = num_data_points / (communicator_size - 1);
    int num_data_points_per_master = num_data_points % (communicator_size - 1);

    // how many raw data points should each process receive
    int* send_receive_count_buffer = (int*) malloc(sizeof(int) * communicator_size);
    send_receive_count_buffer[0] = num_data_points_per_master;
    for (int i = 1; i < communicator_size; i++){
      send_receive_count_buffer[i] = num_data_points_per_worker;
    }

    // data points send buffer
    float** data_points_send_buffer = NULL;
    if (my_rank == 0){
      data_points_send_buffer = (float**) malloc((sizeof(float*) * num_data_points));
      for (int i = 0; i < num_data_points; i++){
        data_points_send_buffer[i] = raw_data_points[i].attributes;
      }
    }

    // data points receive buffer
    my_raw_data_points = (RawDataPoint*) malloc(sizeof(RawDataPoint) * send_receive_count_buffer[my_rank]);
    float** data_points_receive_buffer = (float**) malloc(sizeof(float*) * send_receive_count_buffer[my_rank]);
    for (int i = 0; i < send_receive_count_buffer[my_rank]; i++){
      data_points_receive_buffer[i] = my_raw_data_points[i].attributes;
    }


    // set displacements of data points for each process
    MPI_Aint* displacements_buffer = (MPI_Aint*) malloc(sizeof(MPI_AINT) * communicator_size);
    MPI_Aint start_address;
    MPI_Get_address(data_points_send_buffer[0], &start_address);
    int offset_index = 0;
    for (int i = 0; i < communicator_size; i++){
      MPI_Aint current_address;
      MPI_Get_address(data_points_send_buffer[offset_index], &current_address);
      displacements_buffer[i] = current_address - start_address;
      offset_index += send_receive_count_buffer[i];
    }

    MPI_Scatterv(data_points_send_buffer, send_receive_count_buffer, displacements_buffer, mpi_raw_point_type,
                 data_points_receive_buffer, send_receive_count_buffer[my_rank], mpi_raw_point_type, 0, MPI_COMM_WORLD);

    #if DEBUG
    printf("Rank: %d, Received: %d", my_rank, num_my_data_points);
    #endif

    free(data_points_receive_buffer);
    free(data_points_send_buffer);
    free(send_receive_count_buffer);
    free(displacements_buffer);

    // do work


    // free cluster memory
    for (int i = 0; i < num_clusters; i++)
      free(clusters[i].centroid.attributes);
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
