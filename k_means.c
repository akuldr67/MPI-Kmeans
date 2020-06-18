#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>

int findNearestCenter(double datapoint[],int k, int n, double mu[][n]){

	double w[k]; //distance of datapoint with corresponding means

	for(int i=0;i<k;i++){
		w[i]=0;
		for(int j=0;j<n;j++){
			w[i]+=((datapoint[j]-mu[i][j])*(datapoint[j]-mu[i][j]));
		}
	}
	double min = 9999999;
	int ans = 0;
	for(int i=0;i<k;i++){
		if(w[i]<min){
			min = w[i];
			ans = i;
		}
	}
	return ans;
}


void main(int argc, char *argv[]){

	FILE *fp = fopen("iris.data","r");
	const char s[2] = ",";
	char *token;
	int i,j;
	int count=0;
	int rows=0;
	int k=3;

	double elapsed_time;
	int numprocs, myid,Root=0;
	int remaining_first,local_n,my_first,remaining_length,total_local;

	/*....MPI Initialisation....*/
	MPI_Init(&argc, &argv);
	MPI_Barrier(MPI_COMM_WORLD);
	// elapsed_time = - MPI_Wtime();
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	if(myid == Root){
		if(fp!=NULL){
			for (char c = getc(fp); c != EOF; c = getc(fp)){
	        	if (c == '\n')
	            	count = count + 1; 
	    	}
		}
		fclose(fp);
		fp = fopen("iris.data","r");
		char line[256];
		if(fgets(line, sizeof line, fp)!=NULL){
			token = strtok(line,s);
			i=0;
			while(token!=NULL){
				i++;
				token = strtok(NULL,s);
			}
			rows=i-1;
		}
	}
	fclose(fp);

	MPI_Bcast(&count, 1, MPI_INT, Root, MPI_COMM_WORLD);
	MPI_Bcast(&rows, 1, MPI_INT, Root, MPI_COMM_WORLD);

	double data[count][rows];

	int countIterator = 0;
	fp = fopen("iris.data","r");

	if(fp==NULL)
		printf("can't open dataset\n");
	else{
		if(myid==Root){
			//reading data
			char line[256];
			while(fgets(line, sizeof line, fp)!=NULL){
				//getting token by dividing line over every ','
				token = strtok(line,s);
				
				if(strcmp(line,"\n")!=0){

					i=0;
					//while there are more tokens present in that line
					while(1){
						//getting next token, so that last column could be ignored
						char *token2 = token;
						token = strtok(NULL,s);
						if(token!=NULL){
							float x = atof(token2);
							data[countIterator][i] = x; 	//storing data
							// sum[i]+=x;				//calculating sum for mean
						}
						else break;
						i++;
					}
					//counting no of rows (data enteries)
					countIterator++;

				}
			}
			//printing total no of datapoints and their no of dimensions
			printf("\n Total No of datapoints: %d \n No of dimensions: %d \n\n",count,rows);
			fclose(fp);

			//taking value of no of clusters from user
			printf(" Enter the value of k for k-means clustering: ");
			scanf("%d",&k);
			printf("\n");
			//if entered value of k is even more than total size of data, asking again to enter
			while(k>count){
				printf(" Entered value of k is even more than no of datapoints(%d). Enter again: ",count);
				scanf("%d",&k);
				printf("\n");
			}
		}


		elapsed_time = - MPI_Wtime();

		MPI_Bcast(&k, 1, MPI_INT, Root, MPI_COMM_WORLD);


		// /* ****scaterv*** */

		int sendCounts[numprocs];
		int displs[numprocs];
		int sendCounts2[numprocs];
		int displs2[numprocs];
		local_n = count/numprocs;
		int rem = count%numprocs;

		displs[0] = 0;
		if(rem>0) sendCounts[0] = local_n + 1;
		else sendCounts[0] = local_n;

		for(i=1;i<numprocs;i++){
			displs[i] = displs[i-1]+sendCounts[i-1];
			if(i<rem){
				sendCounts[i] = local_n+1;
			}
			else sendCounts[i] = local_n;
		}

		// if(myid == Root){
		// 	printf("sendCounts:\n");
		// 	for(i=0;i<numprocs;i++)
		// 		printf("%d ", sendCounts[i]);
		// 	printf("\n");
		// 	printf("displs:\n");
		// 	for(i=0;i<numprocs;i++)
		// 		printf("%d ", displs[i]);
		// 	printf("\n");
		// }

		for(i=0;i<numprocs;i++){
			sendCounts2[i] = sendCounts[i];
			displs2[i] = displs[i];
			sendCounts[i]*=rows;
			displs[i]*=rows;
		}

		double local_data[sendCounts2[myid]][rows];

		MPI_Scatterv(data, sendCounts, displs, MPI_DOUBLE, local_data, sendCounts[myid], MPI_DOUBLE, Root, MPI_COMM_WORLD);

		// for(i=0;i<sendCounts2[myid];i++){
		// 	for(j=0;j<rows;j++)
		// 		printf("%d data[%d,%d] -> %.2lf \n", myid, i, j, local_data[i][j]);
		// }

		// /*  ***end*** */


		// /* ***remained one method** */

		// local_n = count/numprocs;
		// my_first = myid * count/numprocs;
		// remaining_first = local_n*numprocs;
		// remaining_length = count-remaining_first;
		// total_local = local_n*rows;

		// double local_data[local_n][rows];
		// double remained_data[remaining_length][rows];

		// MPI_Scatter(data, total_local, MPI_DOUBLE, local_data, total_local, MPI_DOUBLE, Root, MPI_COMM_WORLD);

		// if(myid==Root){
		// 	for(i=remaining_first,j=0;i<count;i++,j++){
		// 		for(int k=0;k<rows;k++)
		// 			remained_data[j][k] = data[i][k];
		// 	}
		// }

		// for(i=0;i<local_n;i++){
		// 	for(j=0;j<rows;j++)
		// 		printf("%d data[%d,%d] -> %.2lf \n", myid, i, j, local_data[i][j]);
		// }

		// /*  ***end*** */


		//intialzation for k means
		double mu[k][rows]; //cluster centers
		int assigned[count]; //each node assigned to which center
		int asgn[k][count]; //which nodes are assigned to the particular cluster
		double j_old = 0.0, j_new = 1000000.0; 	//distortion measures
		// bool changeInClusters = true;	//to check if cluster assignment is changed or not
		int lengths[k];		//stores no of assigned datapoints to the corresponding clusters


		double local_mu[k][rows];
		int local_assigned[sendCounts2[myid]];
		int local_asgn[k][sendCounts2[myid]];
		int local_lengths[k];
		double local_j_old = 0.0, local_j_new = 1000000.0;


		//initialising each local node assigned to local center 0
		for(i=0;i<sendCounts2[myid];i++){
			local_assigned[i] = 0;
			for(j=0;j<k;j++)
				local_asgn[j][i] = 0;
		}

		//initializing cluster centers randomly as equal to first k data points in the root processor
		if(myid==Root){
			for(i=0;i<k;i++){
				for(j=0;j<rows;j++){
					mu[i][j] = data [i][j];
				}
			}
		}

		MPI_Bcast(&mu, k*rows, MPI_DOUBLE, Root, MPI_COMM_WORLD);



		//if there is no change in clusters, than in next iteration new mu will be same and hence J difference will be 0,
		// hence no need to separately check for changeInclusters, 

		while(fabs(j_new-j_old)>0.00000001){

			j_old = j_new;

			//initializing local lengths to 0
			for(i=0;i<k;i++){
				local_lengths[i] = 0;
			}

			//assigning each local data points to a local cluster center (mu)
			for(i=0;i<sendCounts2[myid];i++){
				local_assigned[i] = findNearestCenter(local_data[i],k,rows,mu);
				local_lengths[local_assigned[i]]++;
			}

			//initializing local cluster centers
			for(i=0;i<k;i++){
				for(j=0;j<rows;j++)
					local_mu[i][j]=0.0;
			}

			//updating local cluster centers
			for(i=0;i<sendCounts2[myid];i++){
				for(j=0;j<rows;j++){
					local_mu[local_assigned[i]][j]+=local_data[i][j];
				}
			}

			MPI_Reduce(&local_mu, &mu, k*rows, MPI_DOUBLE, MPI_SUM, Root, MPI_COMM_WORLD);
			MPI_Reduce(&local_lengths, &lengths, k, MPI_INT, MPI_SUM, Root, MPI_COMM_WORLD);

			if(myid==Root){
				for(i=0;i<k;i++){
					for(j=0;j<rows;j++){
						if(lengths[i]==0)
							mu[i][j]=0.0;
						else 
							mu[i][j]/=(lengths[i]*1.0);
					}
				}
			}

			MPI_Bcast(&mu, k*rows, MPI_DOUBLE, Root, MPI_COMM_WORLD);

			//printing cluster centers of current iteration
			// printf(" new cluster centers are: \n");
			// for(i=0;i<k;i++){
			// 	for(j=0;j<rows;j++)
			// 		printf(" %.2f ", mu[i][j]);
			// 	printf("\n");
			// }


			//calculating new local J
			local_j_new = 0.0;
			for(i=0;i<sendCounts2[myid];i++){
				for(j=0;j<rows;j++){
					local_j_new += ((local_data[i][j]-mu[local_assigned[i]][j])*(local_data[i][j]-mu[local_assigned[i]][j]));
				}
			}
			MPI_Reduce(&local_j_new, &j_new, 1, MPI_DOUBLE, MPI_SUM, Root, MPI_COMM_WORLD);
			MPI_Bcast(&j_new, 1, MPI_DOUBLE, Root, MPI_COMM_WORLD);
		}


			//printing assigned cluster value for each datapoint
			// printf(" Final assigned to datapoints: \n");
			// for(i=0;i<count;i++)
			// 	printf("%d ", assigned[i]);
			// printf("\n\n");


			//printing final result: cluster centers and no of assigned datapoints
		if(myid==Root){
			printf(" After k-means clustering, Cluster centers and no of datapoints assigned to them are: \n\n");
			for(i=0;i<k;i++){
				printf("\t %d) \t", i+1);
				for(j=0;j<rows;j++)
					printf(" %.4f ", mu[i][j]);
				printf("\t datapoints: %d \n",lengths[i]);
			}
			printf("\n");
			elapsed_time+=MPI_Wtime();
			printf ("\nTime taken = %f\n", elapsed_time);
		}

	}
	MPI_Finalize();
}
