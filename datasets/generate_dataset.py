"""
A script to generate a dataset
"""

import argparse
import random
import time

def generate_uniform(num_points, num_attributes):
    return [[random.random() for a in range(num_attributes)] for i in range(num_points)]

def generate_clouds(num_points, num_attributes, num_clouds):
    clouds = generate_uniform(num_clouds, num_attributes)
    points = []
    for i in range(num_points):
        cloud = random.choice(clouds)
        point = []
        for ai in range(num_attributes):
            point.append(random.triangular(0,1,cloud[ai]))
        points.append(point)
    return points


def main():
    random.seed(time.time())
    
    parser = argparse.ArgumentParser(description='Generates a dataset for the k-means cluster algorithm')
    parser.add_argument('point_count', type=int, help='number of data points to generate')
    parser.add_argument('attributes_count', type=int, help='number of attributes per point')
    parser.add_argument('filename', type=str, help='name of output file')
    parser.add_argument('--clouds', type=int, default=0, nargs='?', help='type of dataset to be generated: uniform(default) or point clouds')

    args = parser.parse_args()

    print(args)

    points = None
    if args.clouds == 0:
        points = generate_uniform(args.point_count, args.attributes_count)
    else :
        points = generate_clouds(args.point_count, args.attributes_count, args.clouds)

    with open(args.filename, 'w') as file:
        file.write(f'{args.point_count} {args.attributes_count}\n')
        for i in range(args.point_count):
            line = ''
            for ai in range(args.attributes_count):
                line += '{:.3f} '.format(points[i][ai])
            line += '\n'
            file.write(line)

    print(f'Created dataset {args.filename} with {args.point_count} points and {args.attributes_count} attributes')


if __name__ == '__main__':
    main()
