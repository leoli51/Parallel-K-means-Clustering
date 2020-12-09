"""
A script to generate a dataset
"""

import argparse
import random


def main():
    parser = argparse.ArgumentParser(description='Generates a dataset for the k-means cluster algorithm')
    parser.add_argument('point_count', type=int, help='number of data points to generate')
    parser.add_argument('attributes_count', type=int, help='number of attributes per point')
    parser.add_argument('filename', type=str, help='name of output file')

    args = parser.parse_args()

    print(args)

    with open(args.filename, 'w') as file:
        file.write(f'{args.point_count} {args.attributes_count}\n')
        for i in range(args.point_count):
            line = ''
            for ai in range(args.attributes_count):
                line += '{:.3f} '.format(random.random())
            line += '\n'
            file.write(line)

    print(f'Created dataset {args.filename} with {args.point_count} points and {args.attributes_count} attributes')


if __name__ == '__main__':
    main()
