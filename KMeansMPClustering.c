/**
K-Means Clustering with Open MP
**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "KMeansClusteringDefs.h"
#include "KMeansFileUtility.c"
#include <unistd.h>
#include <time.h>
#include <omp.h>

void updateClusters(ClusterDataPoint* data_points, Cluster* clusters, int num_attributes, int num_data_points, int num_clusters);
int assignPointsToNearestClusterSerial(ClusterDataPoint* my_raw_data,Cluster* clusters,int num_attributes,int my_raw_data_num,int num_clusters,_Bool* hasChanged);

int main(int argc, char** argv)
{
  // K-means
  int num_data_points;
  int num_attributes;
  int num_clusters;
  int max_iterations = -1;
  
  //timing
  clock_t start,finish;
  
  // parse file
  char* filename;
  RawDataPoint* raw_data_points;
  if(parseArgs(argc, argv, &filename, &num_clusters, &max_iterations) == -1) return -1;
  if(num_clusters <= 1) { printf("Too few clusters, you should use a minimum of 2 clusters\n"); return -1; }
  if(parseFile(filename, &num_data_points, &num_attributes, &raw_data_points) == -1) return -1;
  if(num_clusters >= num_data_points) { printf("Too few datapoints, they should be more than the clusters\n"); return -1; } //TODO inserire il controllo in parseFile è meglio
  
  start = clock();
  Cluster *clusters = (Cluster*) malloc(sizeof(Cluster) * num_clusters);
  ClusterDataPoint* my_data_points = (ClusterDataPoint*) malloc(sizeof(ClusterDataPoint)*num_data_points);
  int num_iterations = 0;
  #pragma omp parallel
  {
    // initialize clusters as empty
    for (int i = 0; i < num_clusters; i++) 
     {
      clusters[i].centroid.attributes = malloc(sizeof(float)*num_attributes);
      for(int attr = 0; attr < num_attributes; attr++) clusters[i].centroid.attributes[attr] = raw_data_points[i].attributes[attr]; 
      clusters[i].cluster_id = i; 
     }
  
    // initialize ClusterDataPoint 
    for(int i = 0; i < num_data_points; i++)
      my_data_points[i].data_point = raw_data_points[i];
  
    //do work
    _Bool hasChanged;
    do
    {
     hasChanged = 0;
     num_iterations++;
     assignPointsToNearestClusterSerial(my_data_points, clusters, num_attributes, num_data_points, num_clusters, &hasChanged);
     updateClusters(my_data_points, clusters, num_attributes, num_data_points, num_clusters);  
   }while(hasChanged && (max_iterations <= 0 || num_iterations < max_iterations));
  }
 finish = clock(); 
 double elapsed = (double)(finish - start)/CLOCKS_PER_SEC;
 
 int *clusters_id = (int*) malloc(sizeof(int)*num_data_points);
 for(int i = 0; i < num_data_points; i++) clusters_id[i] = my_data_points[i].cluster_id;
 
 char *clusters_filename = "MPClusters.txt";
 
 printMyData(clusters_filename,clusters_id,num_data_points);
 
 free(clusters_id);
 
 char *result = "MPResult.txt";
 printf("Result obtained with %d iterations\n",num_iterations);
 printf("Final centroids written in %s, clusters_id of points written in %s\n",result,clusters_filename);
 printf("Elapsed time: %e seconds\n",elapsed);
 printResult(result,clusters,num_clusters,num_attributes);
    
 
 // free cluster memory
 for (int i = 0; i < num_clusters; i++)
    free(clusters[i].centroid.attributes);
 free(clusters);

 // free data points.
 for (int i = 0; i < num_data_points; i++)
    free(raw_data_points[i].attributes);
    
 free(raw_data_points);
 free(my_data_points);
 return 0;
}

int assignPointsToNearestClusterSerial(ClusterDataPoint* my_raw_data,Cluster* clusters,int num_attributes,int my_raw_data_num,int num_clusters,_Bool* hasChanged)
{
  int i,j,n_attr,cluster_index;
  float distance,min_distance,temp;
  ClusterDataPoint point;
  Cluster cluster;
  #pragma omp for
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
        my_raw_data[i].cluster_id = cluster_index; //assign the nearest cluster to the point TODO (my_raw_data[i] sostitiuibile da point?) -> probabilmente no
        *hasChanged = 1;
      }
   }
  return 0;
}

void updateClusters(ClusterDataPoint* data_points, Cluster* clusters, int num_attributes, int num_data_points, int num_clusters)
{
  float sums[num_clusters][num_attributes];
  int nums[num_clusters];
  #pragma omp for
  for(int i = 0; i < num_clusters; i++)
   {
     nums[i] = 0;
     for(int j = 0; j < num_attributes; j++)
       sums[i][j] = 0;
   }
  ClusterDataPoint point;
  for(int dp = 0; dp < num_data_points; dp++)
    {
      point = data_points[dp];
      for(int attr = 0 ; attr < num_attributes; attr ++)
         sums[point.cluster_id][attr] += point.data_point.attributes[attr];
      nums[point.cluster_id] += 1;
    }
  for(int c = 0; c < num_clusters; c++) 
     for(int i = 0; i < num_attributes; i++)
       if(nums[c] != 0) clusters[c].centroid.attributes[i] = sums[c][i] / nums[c];
       else clusters[c].centroid.attributes[i] = 0;
}
