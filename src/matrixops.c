#include "matrixops.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Function to generate the pseudo-inverse of a random 30x40 matrix
void generatePseudoInverseMatrix(double pseudoInverse[COLS][ROWS]) {
    // Seed the random number generator
    srand(time(NULL));

    // Fill the matrix with arbitrary values (e.g., random values between 0 and 1)
    for (int i = 0; i < COLS; ++i) {
        for (int j = 0; j < ROWS; ++j) {
            pseudoInverse[i][j] = (double)rand() / RAND_MAX;
        }
    }
}

// Utility function to print a matrix
void printMatrix(const double matrix[COLS][ROWS], int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%10.4f ", matrix[i][j]);
        }
        printf("\n");
    }
}

void transposeMatrix(const double A[ROWS][COLS], double B[COLS][ROWS]) {
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            B[j][i] = A[i][j];
        }
    }
}

void multiplyMatrices(const double A[COLS][ROWS], const double B[ROWS][ROWS], double C[COLS][ROWS]) {
    for (int i = 0; i < COLS; ++i) {
        for (int j = 0; j < ROWS; ++j) {
            C[i][j] = 0;
            for (int k = 0; k < ROWS; ++k) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void computeSVD(const double A[ROWS][COLS], double U[ROWS][ROWS], double S[ROWS], double V[COLS][COLS]) {
    double AT[COLS][ROWS];
    transposeMatrix(A, AT);

    double ATA[COLS][COLS] = {0};
    for (int i = 0; i < COLS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            for (int k = 0; k < ROWS; ++k) {
                ATA[i][j] += AT[i][k] * A[k][j];
            }
        }
    }

    // Initialize eigenvectors and eigenvalues
    double eigenvectors[COLS][COLS] = {0};
    double eigenvalues[COLS] = {0};
    for (int i = 0; i < COLS; ++i) {
        eigenvalues[i] = 1.0;
        for (int j = 0; j < COLS; ++j) {
            eigenvectors[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    double diff;
    for (int iter = 0; iter < MAX_ITER; ++iter) {
        for (int i = 0; i < COLS; ++i) {
            double lambda = eigenvalues[i];
            double vi[COLS] = {0};

            // Update vi using power iteration
            double norm = 0.0;
            for (int j = 0; j < COLS; ++j) {
                vi[j] = 0.0;
                for (int k = 0; k < COLS; ++k) {
                    vi[j] += ATA[j][k] * eigenvectors[i][k];
                }
                norm += vi[j] * vi[j];
            }
            norm = sqrt(norm);
            for (int j = 0; j < COLS; ++j) {
                vi[j] /= norm;
            }

            double newLambda = 0.0;
            for (int j = 0; j < COLS; ++j) {
                newLambda += vi[j] * ATA[j][i];
            }

            diff = fabs(newLambda - lambda);
            eigenvalues[i] = newLambda;

            // Update the eigenvector
            for (int j = 0; j < COLS; ++j) {
                eigenvectors[i][j] = vi[j];
            }
        }

        if (diff < EPSILON) {
            break;
        }
    }

    // U is the left singular vectors
    double temp[ROWS][COLS] = {0};
    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            for (int k = 0; k < COLS; ++k) {
                temp[i][j] += A[i][k] * eigenvectors[k][j];
            }
        }
    }

    for (int i = 0; i < ROWS; ++i) {
        for (int j = 0; j < ROWS; ++j) {
            U[i][j] = temp[i][j] / sqrt(eigenvalues[j]);
        }
    }

    // V is the right singular vectors
    for (int i = 0; i < COLS; ++i) {
        for (int j = 0; j < COLS; ++j) {
            V[i][j] = eigenvectors[i][j];
        }
    }

    // S is the diagonal matrix of singular values
    for (int i = 0; i < ROWS; ++i) {
        S[i] = sqrt(eigenvalues[i]);
    }
}

void computePseudoInverse(const double A[ROWS][COLS], double pseudoInverse[COLS][ROWS]) {
    double U[ROWS][ROWS] = {0};
    double S[ROWS] = {0};
    double V[COLS][COLS] = {0};

    computeSVD(A, U, S, V);

    // Create S+ (pseudo-inverse of S)
    double S_plus[ROWS][COLS] = {0};
    for (int i = 0; i < ROWS; ++i) {
        if (fabs(S[i]) > EPSILON) {
            S_plus[i][i] = 1.0 / S[i];
        }
    }

    double VS_plus[COLS][ROWS] = {0};
    multiplyMatrices(V, S_plus, VS_plus);

    transposeMatrix(U, pseudoInverse);
    multiplyMatrices(VS_plus, pseudoInverse, pseudoInverse);
}