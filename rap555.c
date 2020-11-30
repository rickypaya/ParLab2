#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <omp.h>
#include <string.h>

typedef struct{
	int rank;
	int size;
	int ncols;
	int nrows;
	int **grid;
	int **next_grid;
	int generations;
	char *infile;
	char *outfile;
}life_t;

void init_grid(life_t *life){
	FILE *fp;
	int i,j;
	int ncols = life->ncols;
	int nrows = life->nrows;

	life->grid = (int **)malloc(sizeof(int *) * (ncols+2));
	life->next_grid = (int **)malloc(sizeof(int *) * (ncols+2));

	for(i=0;i<ncols+2;i++){
		life->grid[i] = (int *) malloc(sizeof(int) * (nrows+2));
		life->next_grid[i] = (int *) malloc(sizeof(int) * (nrows+2));
	}
	for(i = 0; i<life->ncols+2;i++){
		for(j=0;j<life->nrows+2;j++){
			life->grid[i][j]=0;
			life->next_grid[i][j]=0;
		}
	}
	printf("Init grids, reading file\n");
	fp = fopen(life->infile,"r");
	if(fp==NULL){
		printf("%s",life->infile);
		printf("Infile failed\n");
	}
	int data;
	for(int i = 1; i <= nrows; i++){
			for(int j = 1; j <= ncols; j++){
				fscanf(fp,"%d", &data);
				life->grid[i][j] = data;
			}
		}
	fclose(fp);
	printf("file read\n");
}

void init_life(life_t *life, int*c, char ***v){
	int argc = *c;
	char **argv=*v;
	life->rank = 0;
	life->ncols = atoi(argv[2]);
	life->nrows = atoi(argv[2]);
	life->generations = atoi(argv[1]);

	omp_set_num_threads(atoi(argv[3]));
	life->size = atoi(argv[3]);


	life->infile = argv[4];
	char output[20];
	strcpy(output,life->infile);
	strcat(output,".out");
	life->outfile = output;
	printf("life init, init grid \n");

	init_grid(life);
}

void copy_bounds(life_t *life){
	int i,j;

	int rank = life->rank;
	int size = life->size;
	int ncols = life->ncols;
	int nrows = life->nrows;
	int **grid = life->grid;

	if(size == 1){
		for(j=0;j<nrows+2;j++){
			grid[ncols+1][j]=grid[1][j];
			grid[0][j]=grid[ncols][j];
		}
	}

	for(i = 1; i<=ncols;i++){
		grid[i][0] = grid[1][nrows];
		grid[i][nrows+1]=grid[i][1];
	}
}

void check_cells(life_t *life){
	int i,j,k,l,neighbors;
	int ncols = life->ncols;
	int nrows = life->nrows;

	int **grid =life->grid;
	int **next_grid = life->next_grid;

	#pragma omp parallel private(neighbors,j,k,l)
	for(i = 1; i<=ncols; i++){
		for(j=1;j<=nrows;j++){
			neighbors = 0;

			for(k=i-1;k<=i+1;k++){
				for(l=j-1;l<=j+1;l++){
					if((k!=i && l!=j) && grid[k][l] != 0){
						neighbors++;
					}
				}
			}

			if(neighbors<2||neighbors >3){
				next_grid[i][j] = 0;
			}else if(grid[i][j] != 1 || neighbors == 3){
				next_grid[i][j] = grid[i][j]+1;
			}

		}
	}
}

void update(life_t *life){
	int i,j;
	int ncols = life->ncols;
	int nrows = life->nrows;
	int **grid = life->grid;
	int **next_grid = life->next_grid;

	#pragma omp parallel for private(j)
	for(i = 0; i<ncols+2;i++){
		for(j=0;j<nrows+2;j++){
			grid[i][j] = next_grid[i][j];
		}
	}
}

void finish(life_t *life){
	//writes the final grid to file, clears the grids
	FILE *fp;
	int i,j;
	int ncols = life->ncols;
	int nrows = life->nrows;

	fp = fopen(life->outfile,"w+");

	for(j=1; j<=nrows; j++){
		for(i = 1; i<=ncols; i++){
			fprintf(fp,"%d",life->grid[i][j]);
			
		}
		fprintf(fp,"\n");
	}

	for (i = 0; i < ncols+2; i++) {
		free(life->grid[i]);
		free(life->next_grid[i]);
	}

	free(life->grid);
	free(life->next_grid);

}

void displayGrid(life_t *life){
	int nrows = life->nrows;
	int ncols = life->ncols;
	int **grid = life->grid;

	for(int j = 0; j<=nrows;j++){
		for(int i = 0; i<=ncols;i++){
			printf("%d",grid[i][j]);
		}
		printf("\n");
	}
}

int main(int argc,char ** argv){
	int count;
	life_t life;
	printf("life started\n");

	init_life(&life, &argc, &argv);
	printf("life initialized\n");

	displayGrid(&life);

	for(count = 0; count<life.generations;count++){
		copy_bounds(&life);
		printf("bounds copied %d\n",count);

		check_cells(&life);
		printf("cells checked %d\n",count);

		update(&life);
		printf("cells updated %d\n",count);

		displayGrid(&life)
	}
	
	finish(&life);
	printf("life finished\n");



	return 0;
}









