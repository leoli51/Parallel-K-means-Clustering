/*
* Parallel K-means clustering
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"
#include "KMeansClusteringDefs.h"
#include "KMeansFileUtility.h"
#include <omp.h>
#include <unistd.h>


int main(int argc, char** argv){

    // Standard MPI code
    int communicator_size;
    int my_rank;
    printf("\n");
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &communicator_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //timing values
    double local_start,local_finish,local_elapsed,elapsed;

    // K-means variables
    int num_data_points;
    int num_attributes;
    int num_clusters;
    int max_iterations = -1;

    // arrays of datapoints and clusters
    Cluster* clusters;
    ClusterDataPoint* my_data_points;
    RawDataPoint* raw_data_points;
    RawDataPoint* my_raw_data_points;
    int num_my_data_points;

    // master process parses the command line options and the file
    if (my_rank == 0){
        // parse file
        char* filename;
        if(parseArgs(argc, argv, &filename, &num_clusters, &max_iterations) == -1) return -1;
        if(num_clusters <= 1) { printf("Too few clusters, you should use a minimum of 2 clusters\n"); return -1; }
        if(parseFile(filename, &num_data_points, &num_attributes, &raw_data_points) == -1) return -1;
        if(num_clusters >= num_data_points) { printf("Too few datapoints, they should be more than the clusters\n"); return -1; } 

    }

    MPI_Barrier(MPI_COMM_WORLD);
    local_start = MPI_Wtime();
    
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

    // initialize clusters as empty
    clusters = (Cluster*) malloc(sizeof(Cluster) * num_clusters);
    for (int i = 0; i < num_clusters; i++){
        clusters[i].cluster_id = i;
    }

    // send/receive clusters
    sendClusters(my_rank, num_clusters, num_attributes, clusters, raw_data_points);

    // send/receive points
    sendPoints(my_rank, communicator_size, num_data_points, num_attributes,
               &num_my_data_points,
               raw_data_points, &my_raw_data_points, &my_data_points);
    // do work
    _Bool hasChanged;
    int num_iterations = 0;
    int *num_points_per_cluster = malloc(sizeof(int) * num_clusters);
    do 
    {
      hasChanged = 0;
      num_iterations++;
      
      assignPointsToNearestCluster(my_data_points, clusters, num_attributes, num_my_data_points, num_clusters, &hasChanged,num_points_per_cluster); //we give every point the id of the nearest cluster
      MPI_Allreduce(&hasChanged, &hasChanged, 1, MPI_BYTE, MPI_BOR, MPI_COMM_WORLD); //if someone has changed at least a datapoint cluster_id, hasChanged is 1, otherwise it's 0
      if (!hasChanged) break; //if we haven't moved any datapoint, then we have finished assigning it 
      
      updateLocalClusters(my_rank, num_attributes, num_my_data_points, my_data_points, num_clusters, clusters); //we calculate our local average for new clusters

      synchronizeClusters(my_rank, num_clusters, num_attributes, clusters, num_points_per_cluster); //we reduce the average to get new centroids for the clusters
      
    }while(max_iterations <=0 || num_iterations < max_iterations);
    local_finish = MPI_Wtime();
    local_elapsed = local_finish - local_start;
    MPI_Reduce(&local_elapsed,&elapsed,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD); //get the maximum time of the processes
    
    int *clustered_points = NULL;
    if (my_rank == 0)
        clustered_points = (int*) malloc(sizeof(int) * num_data_points); //array where clustered_points[i] = j means point i has cluster j

    gatherDataPoint(my_rank, my_data_points, num_my_data_points, communicator_size, num_data_points, clustered_points); //we gather all the datapoints cluster id's in clustered_points of rank 0
   
    if(my_rank == 0) //put the results in files
     {
        char *result = "result.txt",*cluster_pos = "clusters.txt";
        printf("Result obtained with %d iterations and written in file %s\n",num_iterations,result);
        printf("Elapsed time: %e seconds\n",elapsed);
        printResult(result,clusters,num_clusters,num_attributes);
        printMyData(cluster_pos, clustered_points, num_data_points);
     }

    //free memory
    free(num_points_per_cluster);
    if(my_rank==0) free(clustered_points);
    
    // free cluster memory
    for (int i = 0; i < num_clusters; i++){
      free(clusters[i].centroid.attributes);
    }
    free(clusters);
    
    // free data points.
    for (int i = 0; i < num_my_data_points; i++){
        free(my_raw_data_points[i].attributes);
    }
   
    free(my_data_points);
    free(my_raw_data_points);

    MPI_Finalize();
    return 0; // Return correct status
}


int gatherDataPoint(int my_rank, ClusterDataPoint* my_points, int num_my_points, int comm_size, int num_data_points, int* clustered_points)
{
    // Get number of datapoints that each worker has
    int* num_data_points_per_worker = NULL;
    if (my_rank == 0)
        num_data_points_per_worker = (int*) malloc(sizeof(int) * comm_size);
    
    MPI_Gather(&num_my_points, 1, MPI_INT, num_data_points_per_worker, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // create clustered point send buffer 
    int* my_clustered_points_buffer = (int*) malloc(sizeof(int) * num_my_points);
    for(int i = 0 ; i < num_my_points; i++) 
        my_clustered_points_buffer[i] = my_points[i].cluster_id;

    // create displacements array
    int* displacements = NULL;
    if (my_rank == 0) {
        displacements = (int*) malloc(sizeof(int) * comm_size);
        int displacement = 0;
        for (int i = 0; i < comm_size; i++){
            displacements[i] = displacement;
            displacement += num_data_points_per_worker[i];
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Gatherv(my_clustered_points_buffer, num_my_points, MPI_INT, clustered_points, num_data_points_per_worker, displacements, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(my_rank == 0) 
     {
      free(displacements);
      free(num_data_points_per_worker);
     }
    free(my_clustered_points_buffer);
   return 0;
}


/**
SendClusters send to all the nodes in MPI the clusters and initialize the first i cluster's centroid to the first i datapoints
**/
int sendClusters(int my_rank, int num_clusters, int num_attributes, Cluster* clusters, RawDataPoint* raw_data_points){
    float *cluster_buffer = (float*) malloc(num_clusters*num_attributes*sizeof(float));
    if(my_rank == 0)
     for(int i = 0; i < num_clusters; i++)
      for(int j = 0; j < num_attributes; j++)
        cluster_buffer[(i*num_attributes)+j] = raw_data_points[i].attributes[j];
  
    MPI_Bcast(cluster_buffer, num_clusters*num_attributes, MPI_FLOAT, 0, MPI_COMM_WORLD);
     
    for (int i = 0; i < num_clusters; i++)
    {
        clusters[i].centroid.attributes = malloc(sizeof(float)*num_attributes);
        for(int attr = 0; attr < num_attributes; attr++) clusters[i].centroid.attributes[attr] = cluster_buffer[(i*num_attributes)+attr];
    }
    
    free(cluster_buffer);
  return 0;
}






/**
sendPoints divide the datapoints among the MPI nodes
**/
int sendPoints( int my_rank, int communicator_size, int num_data_points, int num_attributes,
                int* num_my_data_points,
                RawDataPoint* raw_data_points, RawDataPoint** my_raw_data_points, ClusterDataPoint** my_data_points){

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

    // data points send buffer
    float* data_points_send_buffer = NULL;
    if (my_rank == 0){
        data_points_send_buffer = (float*) malloc(sizeof(float) * num_data_points * num_attributes);
        for (int i = 0; i < num_data_points; i++){
            for (int j = 0; j < num_attributes; j++){
                data_points_send_buffer[i*num_attributes + j] = raw_data_points[i].attributes[j];
            }
        }
    }

    (*num_my_data_points) = send_receive_count_buffer[my_rank] / num_attributes;

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

    // assign data points
    (*my_raw_data_points) = (RawDataPoint*) malloc(sizeof(RawDataPoint) * (*num_my_data_points));
    (*my_data_points) = (ClusterDataPoint*) malloc(sizeof(ClusterDataPoint) * (*num_my_data_points));
    int offset = 0;
    for (int i = 0; i < *num_my_data_points; i++){
        (*my_raw_data_points)[i].attributes = (float*) malloc(sizeof(float) * num_attributes);
        memcpy((*my_raw_data_points)[i].attributes, &data_points_receive_buffer[offset], num_attributes * sizeof(float));
        offset += num_attributes;

        (*my_data_points)[i].data_point = (*my_raw_data_points)[i];
    }

    free(data_points_receive_buffer);
    if(my_rank == 0) 
     {
      free(data_points_send_buffer);
      free(displacements_buffer);
     }
    free(send_receive_count_buffer);
  return 0;
   
}





/**
synchronizeClusters calculates the new centroids for the clusters by reducing the part of the average each MPI node calculated
**/
int synchronizeClusters(int my_rank, int num_clusters, int num_attributes, Cluster* clusters, int* points_per_cluster){
    // Allreduce new cluster values
    float* clusters_send_buffer = (float*) malloc(sizeof(float) * num_clusters * num_attributes);
    for (int i = 0; i < num_clusters; i++){
       for(int j = 0; j < num_attributes; j++)
         clusters_send_buffer[(i*num_attributes)+j] = clusters[i].centroid.attributes[j];
        //memcpy(&clusters_send_buffer[i * num_attributes], clusters[i].centroid.attributes, sizeof(float) * num_attributes);
    }

    float* clusters_receive_buffer = (float*) malloc(sizeof(float) * num_clusters * num_attributes);

    MPI_Allreduce(clusters_send_buffer, clusters_receive_buffer, num_clusters * num_attributes, MPI_FLOAT, MPI_SUM, MPI_COMM_WORLD);

    // Allreduce points per cluster
    int* points_per_cluster_receive_buffer = (int*) malloc(sizeof(int) * num_clusters);
    for (int i = 0; i < num_clusters; i++)
        points_per_cluster_receive_buffer[i] = 0;
    
    MPI_Allreduce(points_per_cluster, points_per_cluster_receive_buffer, num_clusters, MPI_INT, MPI_SUM, MPI_COMM_WORLD);


    // Correctly update all clusters
    for (int i = 0; i < num_clusters; i++){
        memcpy(clusters[i].centroid.attributes, &clusters_receive_buffer[i*num_attributes], sizeof(float) * num_attributes);
        for (int j = 0; j < num_attributes; j++)
            clusters[i].centroid.attributes[j] /= points_per_cluster_receive_buffer[i];
    }

    free(clusters_send_buffer);
    free(clusters_receive_buffer);
    free(points_per_cluster_receive_buffer);
    return 0;
}





/**
updateLocalClusters calculate for each cluster the part of the average of the node
**/
int updateLocalClusters(int my_rank, int num_attributes, int num_my_data_points, ClusterDataPoint* my_data_points, int num_clusters, Cluster* clusters){
    for (int i = 0; i < num_clusters; i++){
        for (int j = 0; j < num_attributes; j++)
            clusters[i].centroid.attributes[j] = 0;
    }
    for (int i = 0; i < num_my_data_points; i++){
        for (int j = 0; j < num_attributes; j++){
            clusters[my_data_points[i].cluster_id].centroid.attributes[j] += my_data_points[i].data_point.attributes[j];
        }
    }
   return 0;
}





/**
assignPointsToNearestCluster gives each datapoint a cluster_id, in particular, the cluster_id of the nearest cluster of the point
**/
int assignPointsToNearestCluster(ClusterDataPoint* my_raw_data,Cluster* clusters,int num_attributes,int my_raw_data_num,int num_clusters,_Bool* hasChanged,int* num_points_per_cluster)
{
  int i,j,n_attr,cluster_index;
  float distance,min_distance,temp;
  ClusterDataPoint point;
  Cluster cluster;
  for(i = 0; i < num_clusters ; i++) num_points_per_cluster[i] = 0;
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
     if(my_raw_data[i].cluster_id != cluster_index)
      {
        my_raw_data[i].cluster_id = cluster_index; //assign the nearest cluster to the point 
        *hasChanged = 1;
      }
     num_points_per_cluster[cluster_index] += 1;
   }
  return 0;
}
