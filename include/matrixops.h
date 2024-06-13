#ifndef MATRIXOPS_H
#define MATRIXOPS_H

#define ROWS 30
#define COLS 40
#define EPSILON 1e-10
#define MAX_ITER 1000

void transposeMatrix(const double A[ROWS][COLS], double B[COLS][ROWS]);
void multiplyMatrices(const double A[COLS][ROWS], const double B[ROWS][ROWS], double C[COLS][ROWS]);
void computeSVD(const double A[ROWS][COLS], double U[ROWS][ROWS], double S[ROWS], double V[COLS][COLS]);
void computePseudoInverse(const double A[ROWS][COLS], double pseudoInverse[COLS][ROWS]);
void generatePseudoInverseMatrix(double pseudoInverse[COLS][ROWS]);
void printMatrix(const double matrix[COLS][ROWS], int rows, int cols);

#endif // MATRIXOPS_H