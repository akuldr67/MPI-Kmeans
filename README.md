# MPI_Kmeans
To perform k-means clustering and parallelizing it through MPI.
 Value of k is asked from user.
 iris.data is given as a dataset. Read the data from file and ignore the last column of each row in dataset.

To compile:
```
mpicc -o k_means k_means.c
```
where k_means is the name of the object file.

To run:
```
mpiexec -n 4 ./k_means
```
where 4 is the number of processors. You can give any integer value greater than 1. But generally machines have 4 processors, so giving value of n greater 4 will create lot of overhead and actually increase the time taken.
