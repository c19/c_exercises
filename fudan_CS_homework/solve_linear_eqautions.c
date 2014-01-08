#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int lookfor_notzero(int imax, int jmax, double matrix[imax][jmax], int i, int j){
	for (; i < imax; ++i)
	{//*(matrix + sizeof(double)*i*jmax + sizeof(double)*j)
		if (matrix[i][j] != 0) return i;
	}
	return -1;
}

void swap_row(double *row1, double *row2, int length){
	double tmp;
	for (int i = 0; i < length; ++i)
	{
		tmp = row1[i];
		row1[i] = row2[i];
		*row2 = tmp;
	}
}

int make_pivit_notzero(int imax, int jmax, double matrix[imax][jmax], int i, int j){
	if (matrix[i][j] == 0)
	{
		int s = lookfor_notzero(imax, jmax, matrix, i+1, j);
		if (s > 0){
			swap_row(matrix[i], matrix[s], jmax);
		}else{
			printf("pivit i:%d,j:%d seems to be zero and unswitchable\n", i, j);
		}
	}else{
		return 0;
	}
}

void wipe_zero(int imax, int jmax, double matrix[imax][jmax]){
	for (int i = 0; i < imax; ++i)
	{
		for (int j = 0; j < jmax; ++j)
		{
			if (fabs(matrix[i][j]) < 1.0e-10)
				matrix[i][j] = 0;
		}
	}
}

//row2 = k*row1 + row2
int row_add(double *row1, double *row2, double k, int imax){
	for (int i = 0; i < imax; ++i)
	{
		row2[i] = k*row1[i] + row2[i];
	}
}

int wipe_up(int imax, int jmax, double matrix[imax][jmax], int i, int j){
	for (int up = i-1; up >= 0; --up)
	{
		row_add(matrix[i], matrix[up], -matrix[up][j]/matrix[i][j], jmax);
	}
}

int wipe_below(int imax, int jmax, double matrix[imax][jmax], int i, int j){
	for (int below=i+1; below < imax; ++below)
	{
		row_add(matrix[i], matrix[below], -matrix[below][j]/matrix[i][j], jmax);
	}
}

void pirnt_matrix(int imax, int jmax, double matrix[imax][jmax]){
	for (int i = 0; i < imax; ++i)
	{
		for (int j = 0; j < jmax; ++j)
		{
			printf("%10lf, ", matrix[i][j]);
		}
		printf("\n");
	}
}

int main(int argc, char const *argv[])
{
	///*
	#define ROW 10
	#define COLUMN 11
	double a[ROW][COLUMN-1]={0.638796, 0.736510, 0.827915, 0.714243, 0.580313, 0.502711, 0.320619, 0.381693, 0.829783, 0.090271, 
							0.086381, 0.012368, 0.926026, 0.685283, 0.442655, 0.771235, 0.161726, 0.373213, 0.314419, 0.017243, 
							0.230404, 0.583643, 0.539237, 0.362877, 0.574665, 0.587253, 0.052889, 0.797725, 0.540768, 0.231177, 
							0.321209, 0.695746, 0.469330, 0.033947, 0.543901, 0.688743, 0.981675, 0.360366, 0.065075, 0.064145, 
							0.409384, 0.006006, 0.511788, 0.898087, 0.630414, 0.134535, 0.695637, 0.556015, 0.267766, 0.504105, 
							0.106553, 0.521463, 0.941638, 0.001127, 0.889229, 0.985234, 0.710775, 0.408030, 0.355768, 0.233783, 
							0.716173, 0.837311, 0.067652, 0.918376, 0.541687, 0.839058, 0.234658, 0.968085, 0.053723, 0.984997, 
							0.834031, 0.057627, 0.162390, 0.344131, 0.976396, 0.034571, 0.785434, 0.697810, 0.443574, 0.605635, 
							0.706559, 0.381556, 0.773771, 0.226483, 0.392380, 0.071833, 0.798492, 0.754875, 0.561998, 0.478654, 
							0.744680, 0.519814, 0.721342, 0.365413, 0.330053, 0.084206, 0.169013, 0.143299, 0.970202, 0.799632
							};
	double b[ROW]={0.368936, 0.651629, 0.072565, 0.125020, 0.177600, 0.299796, 0.822566, 0.687749, 0.946997, 0.731359 };
	double x[ROW]={2.496575, -0.639090, 1.453365, -0.661451, -0.029772, 0.233489, -1.001369, 0.120202, -1.591228, 0.104728 };
	/*//*
	#define ROW 2
	#define COLUMN 3
	double a[ROW][COLUMN-1] = {4, 2,
							   9, 7};
	double b[ROW] = {6, 3};
	//*/

	//turn into a extended matrix
	double matrix[ROW][COLUMN];
	for (int i = 0; i < ROW; ++i)
	{
		for (int j = 0; j < COLUMN; ++j)
		{
			matrix[i][j] = j!=COLUMN-1 ? a[i][j] : b[i];
		}
	}
	pirnt_matrix(ROW, COLUMN, matrix);
	printf("==========turn into a upper triangular matrix============\n");
	//turn into a upper triangular matrix
	for (int i = 0; i < ROW; ++i)
	{
		make_pivit_notzero(ROW, COLUMN, matrix, i, i);
		wipe_below(ROW, COLUMN, matrix, i, i);
	}
	//wipe_zero(ROW, COLUMN, matrix);
	pirnt_matrix(ROW, COLUMN, matrix);
	printf("==========echelon_form============\n");
	//solve for x
	double solution[ROW];
	for (int i = ROW - 1; i >= 0; --i)
	{
		wipe_up(ROW, COLUMN, matrix, i, i);
	}
	//wipe_zero(ROW, COLUMN, matrix);
	pirnt_matrix(ROW, COLUMN, matrix);
	printf("==========solution============\n");
	for (int i = 0; i < ROW; ++i)
	{
		solution[i] = matrix[i][COLUMN-1]/matrix[i][i];
		printf("%10lf, ", solution[i]);
	}
	printf("\n");
	return 0;
}

