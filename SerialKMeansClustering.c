/**
Serial K-Means Clustering
**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "KMeansClusteringDefs.h"
#include "KMeansFileUtility.c"
#include <unistd.h>

void updateClusters(ClusterDataPoint* data_points, Cluster* clusters, int num_attributes, int num_data_points, int num_clusters);

int main(int argc, char** argv)
{
  // K-means
  int num_data_points;
  int num_attributes;
  int num_clusters;
  int max_iterations = -1;
  
  // parse file
  char* filename;
  RawDataPoint* raw_data_points;
  if(parseArgs(argc, argv, &filename, &num_clusters, &max_iterations) == -1) return -1;
  if(num_clusters <= 1) { printf("Too few clusters, you should use a minimum of 2 clusters\n"); return -1; }
  if(parseFile(filename, &num_data_points, &num_attributes, &raw_data_points) == -1) return -1;
  if(num_clusters >= num_data_points) { printf("Too few datapoints, they should be more than the clusters\n"); return -1; } //TODO inserire il controllo in parseFile è meglio
  
  // initialize clusters as empty
  Cluster* clusters = (Cluster*) malloc(sizeof(Cluster) * num_clusters);
  for (int i = 0; i < num_clusters; i++) 
   {
     clusters[i].centroid.attributes = malloc(sizeof(float)*num_attributes);
     for(int attr = 0; attr < num_attributes; attr++) clusters[i].centroid.attributes[attr] = raw_data_points[i].attributes[attr]; 
     clusters[i].cluster_id = i; 
   }
  
  // initialize ClusterDataPoint
  ClusterDataPoint* my_data_points = (ClusterDataPoint*) malloc(sizeof(ClusterDataPoint)*num_data_points);
  for(int i = 0; i < num_data_points; i++)
     my_data_points[i].data_point = raw_data_points[i];
  
  //do work
  _Bool hasChanged;
  int num_iterations = 0;
  do
  {
    hasChanged = 0;
    num_iterations++;
    printf("Assignment %d:\n",num_iterations);
    assignPointsToNearestCluster(my_data_points, clusters, num_attributes, num_data_points, num_clusters, &hasChanged);
    
    updateClusters(my_data_points, clusters, num_attributes, num_data_points, num_clusters);
    
  }while(hasChanged && (max_iterations <= 0 || num_iterations < max_iterations));
  
 printf("Result obtained with %d iterations:\n",num_iterations);
 for(int dp = 0; dp < num_data_points; dp++)
    printf("DataPoint %d belongs to cluster %d\n",dp,my_data_points[dp].cluster_id);
    
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

int assignPointsToNearestCluster(ClusterDataPoint* my_raw_data,Cluster* clusters,int num_attributes,int my_raw_data_num,int num_clusters,_Bool* hasChanged)
{
  int i,j,n_attr,cluster_index;
  float distance,min_distance,temp;
  ClusterDataPoint point;
  Cluster cluster;
  //#pragma omp parallel for
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
 	  printf("Point %d is changing cluster from %d to %d because %f < %f\n",i,cluster_index,j,distance,min_distance);
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
       clusters[c].centroid.attributes[i] = sums[c][i] / nums[c];
}
