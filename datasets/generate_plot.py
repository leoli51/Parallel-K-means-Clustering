import matplotlib.pyplot as plot
import argparse
import numpy as np

def distance(p1, p2):
    dst = 0
    for i in range(len(p1)):
        dst += (p1[i] - p2[i])**2
    return dst

def assign_points_to_clusters(points, clusters):
    Ys = [[] for i in range(len(clusters))]
    for point in points:
        min_dst = distance(point, clusters[0])
        cluster_index = 0
        for i, cluster in enumerate(clusters):
            dst = distance(point, cluster)
            if dst < min_dst:
                min_dst = dst 
                cluster_index = i
        Ys[cluster_index].append(point)
    return Ys



def main():
    parser = argparse.ArgumentParser(description='Generates a plot for the k-means cluster algorithm')
    parser.add_argument('points_filename', type=str, help='name of points file')
    parser.add_argument('clusters_filename', type=str, help='name of clusters file')
    args = parser.parse_args()

    print(args)

    with open(args.points_filename, 'r') as points_file:
        points = []
        for index, line in enumerate(points_file.readlines()):
            if index == 0: 
                continue
            
            point = list(map(float, line.split()))
            points.append(point)
    
    with open(args.clusters_filename, 'r') as cluster_file:
        clusters = []
        for line in cluster_file.readlines():
            cluster = list(map(float, line.split()))
            clusters.append(cluster)
    
    cluster_points = map(np.array, assign_points_to_clusters(points, clusters))
    for cluster in cluster_points:
        cluster.T[0]
        plot.scatter(cluster.T[0], cluster.T[1])
    plot.show()


if __name__ == '__main__':
    main()

