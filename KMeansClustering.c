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
        
        parseArgs(argc, argv, &filename, &clusters_size, &max_iterations);
        parseFile(filename, &data_points_size, &attributes_size, &raw_data_points);
    }

    // send/receive information to create datatypes
    int information_buffer[4] = {data_points_size, attributes_size, clusters_size, max_iterations};
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
    createMPIRawDataPoint(&mpi_raw_point_type, attributes_size);
    MPI_Type_commit(mpi_raw_point_type);

    
    // initialize clusters
    clusters = (Cluster*) malloc(sizeof(Cluster) * clusters_size);
    for (int i = 0; i < clusters_size; i++){
        clusters[i].cluster_id = i;
    }

    // initialize cluster buffer
    float** cluster_buffer = (float**) malloc(clusters_size*sizeof(float*));
    for (int i = 0; i < clusters_size; i++){
        cluster_buffer[i] = (float*) malloc(sizeof(float) * attributes_size);
        if (my_rank == 0)
          memcpy(cluster_buffer[i], raw_data_points[i].attributes, sizeof(float) * attributes_size); 
    }

    // send/receive cluster information
    MPI_Bcast(cluster_buffer, clusters_size, mpi_raw_point_type, 0, MPI_COMM_WORLD);

    // assign buffer attributes to cluster struct
    for (int i = 0; i < clusters_size; i++){
        clusters[i].centroid.attributes = cluster_buffer[i];
    }

    #if DEBUG
    printf("Rank:%d, Clusters:\n");
    for (int i = 0; i < clusters_size; i++){
      printf("Cluster %d: ", clusters[i].cluster_id);
      for (int j = 0; j < attributes_size; j++)
        printf("%f", clusters[i].centroid.attributes[j]);
      printf("\n");
    }
    #endif

    // send data points
    int num_data_points_per_worker = num_data_points / (communicator_size - 1);
    int num_data_points_per_master = num_data_points % (communicator_size - 1);

    // send 
    // how many raw data points should each process receive
    int* send_receive_count_buffer = (int*) malloc(sizeof(int) * communicator_size);
    send_receive_count_buffer[0] = num_data_points_per_master;
    for (int i = 1; i < communicator_size; i++){
      send_receive_count_buffer[i] = num_data_points_per_worker;
    }

    // set displacements of data points for each process
    MPI_Aint* displacements_buffer = (MPI_AINT*) malloc(sizeof(MPI_AINT) * communicator_size);
    MPI_Aint start_address;
    MPI_Get_address(raw_data_points[0], &start_address);
    int offset_index = 0;
    for (int i = 0; i < communicator_size; i++){
      MPI_Aint current_address;
      MPI_Get_address(raw_data_points[offset_index], &current_address);
      displacements_buffer[i] = current_address - start_address;
      offset_index += send_receive_count_buffer[i];
    }

    // prepare receive buffer
    my_raw_data_points = (RawDataPoint*) malloc(sizeof(RawDataPoint) * send_receive_count_buffer[my_rank]);

    MPI_Scatterv(raw_data_points, send_receive_count_buffer, displacements_buffer, mpi_raw_point_type, 
                 my_raw_data_points, send_receive_buffer[my_rank], mpi_raw_data_point_type, 0, MPI_COMM_WORLD);

    #if DEBUG
    printf("Rank: %d, Received: %d", my_rank, num_my_data_points);
    #endif

    free(send_receive_count_buffer);
    free(displacements_buffer);

    // do work

    // free cluster memory
    for (int i = 0; i < clusters_size; i++)
      free(clusters[i].attributes);
    free(clusters);

    // free data points.


    MPI_Finalize();
    return 0; // Return correct status 
}

