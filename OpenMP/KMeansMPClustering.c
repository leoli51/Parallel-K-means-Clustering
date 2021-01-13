/**
K-Means Clustering with Open MP
**/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "KMeansClusteringDefs.h"
#include "KMeansFileUtility.h"
#include <unistd.h>
#include <time.h>
#include <omp.h>


int main(int argc, char** argv)
{

  printf("\nsize of datapoint: %d\n", sizeof(ClusterDataPoint));

  // K-means
  int num_data_points;
  int num_attributes = NUM_ATTRIBUTES;
  int num_clusters;
  int max_iterations = -1;
  
  //timing
  clock_t start,finish;
  
  // parse file
  char* filename;
  ClusterDataPoint* data_points;
  if(parseArgs(argc, argv, &filename, &num_clusters, &max_iterations) == -1) return -1;
  if(num_clusters <= 1) { printf("Too few clusters, you should use a minimum of 2 clusters\n"); return -1; }
  if(parseFile(filename, &num_data_points, &num_attributes, &data_points) == -1) return -1;
  if(num_clusters >= num_data_points) { printf("Too few datapoints, they should be more than the clusters\n"); return -1; } //TODO inserire il controllo in parseFile è meglio
  
  start = clock();

  ClusterDataPoint *clusters = (ClusterDataPoint*) malloc(sizeof(ClusterDataPoint) * num_clusters);

  for (int i = 0; i < num_clusters; i++) 
    {
    for(int attr = 0; attr < num_attributes; attr++) clusters[i].attributes[attr] = data_points[i].attributes[attr]; 
    clusters[i].cluster_id = i; 
    }


  //do work
  int changed = 0;
  int num_iterations = 0;
  float cluster_sums[num_clusters][num_attributes];
  int points_per_cluster[num_clusters];

  #pragma omp parallel num_threads(4)
  {
  do
  {
    #pragma omp barrier
    #pragma omp single
    {
      changed = 0;
      num_iterations++;
    }

    int local_cluster_changes = 0;

    #pragma omp for schedule(static, 4)
    for(int i = 0; i < num_data_points; i++) //for every point to analyze
    {
      float min_distance = -1;
      int cluster_index = -1;
      for(int j = 0; j < num_clusters; j++) //for every cluster
      {
        float distance = 0;
        for(int n_attr = 0; n_attr < num_attributes; n_attr++) //calculate the distance between the point and the cluster
        {
          float temp = (data_points[i].attributes[n_attr] - clusters[j].attributes[n_attr]);
          distance += temp * temp;
        }
        if(j == 0 || distance < min_distance) //if it is the first cluster or it is the nearest up to now
        {
          min_distance = distance; //consider it as the nearest
          cluster_index = j;
        }
      }
      if(data_points[i].cluster_id != cluster_index)
      {
        data_points[i].cluster_id = cluster_index; //assign the nearest cluster to the point TODO (my_raw_data[i] sostitiuibile da point?) -> probabilmente no
        local_cluster_changes++;
      }
    }
    
    #pragma omp atomic
    changed += local_cluster_changes;      

    #pragma omp for
    for(int i = 0; i < num_clusters; i++)
    {
      points_per_cluster[i] = 0;
      for(int j = 0; j < num_attributes; j++)
        cluster_sums[i][j] = 0;
    }

    float local_cluster_sums[num_clusters][num_attributes];
    int local_points_per_cluster[num_clusters];

    for(int i = 0; i < num_clusters; i++)
    {
      local_points_per_cluster[i] = 0;
      for(int j = 0; j < num_attributes; j++)
        local_cluster_sums[i][j] = 0;
    }

    #pragma omp for
    for(int i = 0; i < num_data_points; i++)
    {
      for(int attr = 0 ; attr < num_attributes; attr ++)
        local_cluster_sums[data_points[i].cluster_id][attr] += data_points[i].attributes[attr];
      local_points_per_cluster[data_points[i].cluster_id]++;
    }
  
    #pragma omp for reduction(+: cluster_sums, points_per_cluster)
    for (int c = 0; c < num_clusters; c++){
      points_per_cluster[c] += local_points_per_cluster[c];
      for (int a = 0; a < num_attributes; a++)
        cluster_sums[c][a] += local_cluster_sums[c][a];
    } 

    #pragma omp for collapse(2)
    for(int c = 0; c < num_clusters; c++) 
      for(int i = 0; i < num_attributes; i++)
        if(points_per_cluster[c] != 0) clusters[c].attributes[i] = cluster_sums[c][i] / points_per_cluster[c];
        else clusters[c].attributes[i] = 0;    
    
    } while(changed && (max_iterations <= 0 || num_iterations < max_iterations));
  }

 finish = clock(); 
 double elapsed = (double)(finish - start)/CLOCKS_PER_SEC;
 
 int *clusters_id = (int*) malloc(sizeof(int)*num_data_points);
 for(int i = 0; i < num_data_points; i++) clusters_id[i] = data_points[i].cluster_id;
 
 char *clusters_filename = "MPClusters.txt";
 
 printMyData(clusters_filename,clusters_id,num_data_points);
 
 free(clusters_id);
 
 char *result = "MPResult.txt";
 printf("Result obtained with %d iterations\n",num_iterations);
 printf("Final centroids written in %s, clusters_id of points written in %s\n",result,clusters_filename);
 printf("Elapsed time: %e seconds\n",elapsed);
 printResult(result,clusters,num_clusters,num_attributes);
    

 free(clusters);
 free(data_points);

 return 0;
}

