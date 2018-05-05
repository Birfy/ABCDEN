/////////AB2C4 dendritic triblock copolymer //////
///////Apr 2018 //////
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include </export/home/cdy/fftw3/include/fftw3.h>
#include </data2/opt/include/gsl/gsl_blas.h>
#include </data2/opt/include/gsl/gsl_linalg.h>
#define MaxIT 50000 //Maximum iteration steps

#define Pi 3.141592653589

#define N_hist 50
/* Parameters used in Anderson convergence */
#define Del(k, i, n) del[(i) + NxNyNz * (n) + N_hist * NxNyNz * (k)]
#define Outs(k, i, n) outs[(i) + NxNyNz * (n) + N_hist * NxNyNz * (k)]
#define U(n, m) up[(m - 1) + (N_rec - 1) * (n - 1)]
#define V(n) vp[n - 1]
#define A(n) ap[n - 1]

double freeE(double *wA, double *wB, double *wC, double *phA, double *phB, double *phC, double *eta);
double getConc(double *phlA, double *phlB, double *phlC, double phs0, double *wA, double *wB, double *wC);
void sovDifFft(double *g, double *w, double *qInt, double z, int ns, int sign, double epk);
void write_ph(double *phA, double *phB, double *phC, double *wA, double *wB, double *wC);
double error_cal(double *waDiffs, double *wbDiffs, double *wcDiffs, double *wAs, double *wBs, double *wCs);
void update_flds_hist(double *waDiff, double *wbDiff, double *wcDiff, double *wAnew, double *wBnew, double *wCnew, double *del, double *outs);
void Anderson_mixing(double *del, double *outs, int N_rec, double *wA, double *wB, double *wC);

void init(int in, double *wA, double *wB, double *wC);
void initW_Random(double *wA, double *wB, double *wC);
void initW_csA15(double *wA, double *wB, double *wC);
void initW_csBCC(double *wA, double *wB, double *wC);
void initW_csFCC(double *wA, double *wB, double *wC);
void initW_csPS(double *wA, double *wB, double *wC);
void initW_csC(double *wA, double *wB, double *wC);
void initW_LAM2(double *wA, double *wB, double *wC);
void initW_csHelix(double *wA, double *wB, double *wC);
void initW_RC(double *wA, double *wB, double *wC);
void initW_H1C(double *wA, double *wB, double *wC);
void initW_aPS(double *wA, double *wB, double *wC);
void initW_csHelix2(double *wA, double *wB, double *wC);

int ZDIMM, NsA, NsB, NsC, NsB1, NsB2, NsB3;
double lx, ly, lz, ds0;
double hAB, fA, fB, fC, hAC, hBC;
double epA, epB, epC;
int Nx, Ny, Nz, NyNz, NxNyNz, Nzh1, Nz2, NxNyNz1;
double *kxyz, dx, dy, dz;
int andmix, andmixsteps;
double wopt, wcmp, errMax;
double fAinit, fBinit, fCinit;
int aismatrix;

char FEname[30], phname[30];

int main(int argc, char **argv)
{
	double *wA, *wB, *wC, *eta, *phA, *phB, *phC;
	double rjk, yj, zk;
	int i, i1, j, k, in, iseed = -3; //local_x_starti;
	double lylx, lzlx;
	long ijk, ijk0;

	//MPI_Status status
	FILE *fp;
	time_t ts;
	iseed = time(&ts);
	srand48(iseed);

	//////////put in para ///////////////////////
	fp = fopen("para", "r");
	fscanf(fp, "%d", &in);
	fscanf(fp, "%d, %d", &andmix, &andmixsteps);
	fscanf(fp, "%lf, %lf", &wopt, &wcmp);
	fscanf(fp, "%lf", &errMax);
	fscanf(fp, "%lf, %lf", &lylx, &lzlx);
	fscanf(fp, "%d", &aismatrix);
	fscanf(fp, "%lf, %lf, %lf", &hAB, &hBC, &hAC);
	fscanf(fp, "%lf, %lf, %lf", &fA, &fB, &fC);
	fscanf(fp, "%lf, %lf, %lf", &lx, &ly, &lz);
	fscanf(fp, "%d, %d, %d", &Nx, &Ny, &Nz);
	fscanf(fp, "%s", FEname); //output file name for parameters;
	fscanf(fp, "%s", phname); //output file name for configuration;
	fscanf(fp, "%lf", &ds0);
	fscanf(fp, "%lf, %lf, %lf", &epA, &epB, &epC);
	fclose(fp);

	NyNz = Ny * Nz;
	NxNyNz = Nx * Ny * Nz;

	Nzh1 = Nz / 2 + 1;
	Nz2 = 2 * Nzh1;
	NxNyNz1 = Nx * Ny * Nzh1;
	double kx[Nx], ky[Ny], kz[Nz];

	wA = (double *)malloc(sizeof(double) * NxNyNz);
	wB = (double *)malloc(sizeof(double) * NxNyNz);
	wC = (double *)malloc(sizeof(double) * NxNyNz);
	phA = (double *)malloc(sizeof(double) * NxNyNz);
	phB = (double *)malloc(sizeof(double) * NxNyNz);
	phC = (double *)malloc(sizeof(double) * NxNyNz);
	eta = (double *)malloc(sizeof(double) * NxNyNz);
	kxyz = (double *)malloc(sizeof(double) * NxNyNz);

	if (lylx != 1)
		ly = lx * sqrt(lylx);
	if (lzlx != 1)
		lz = lx * sqrt(lzlx);
	dx = lx / Nx;
	dy = ly / Ny;
	dz = lz / Nz;
	fC = 1.0 - fB - fA;

	printf("hAB = %.3lf, hAC = %.3lf, hBC = %.3lf\n", hAB, hAC, hBC);
	printf("fA = %.3lf, fB = %.3lf, fC = %.3lf\n", fA, fB, fC);
	printf("dx = %.3lf, dy = %.3lf, dz = %.3lf\n", dx, dy, dz);

	fB = 0.5 * fB;
	fC = 0.25 * fC;

	NsA = ((int)(fA / ds0 + 1.0e-6));
	NsB = ((int)(fB / ds0 + 1.0e-6));
	NsC = ((int)(fC / ds0 + 1.0e-6));

	fp = fopen(FEname, "w");
	fprintf(fp, "Nx = %d, Ny = %d, Nz = %d\n", Nx, Ny, Nz);
	fprintf(fp, "hAB = %lf, hAC = %lf, hBC = %lf\n", hAB, hAC, hBC);
	fprintf(fp, "fA = %lf, fB = %lf, fC = %lf\n", fA, fB, fC);
	fprintf(fp, "NsA = %d, NsB = %d, NsC = %d\n", NsA, NsB, NsC);
	fprintf(fp, "dx = %.6lf, dy = %.6lf, dz = %.6lf\n", dx, dy, dz);
	fclose(fp);

	for (i = 0; i <= Nx / 2 - 1; i++)
		kx[i] = 2 * Pi * i * 1.0 / Nx / dx;
	for (i = Nx / 2; i < Nx; i++)
		kx[i] = 2 * Pi * (i - Nx) * 1.0 / dx / Nx;
	for (i = 0; i < Nx; i++)
		kx[i] *= kx[i];

	for (i = 0; i <= Ny / 2 - 1; i++)
		ky[i] = 2 * Pi * i * 1.0 / Ny / dy;
	for (i = Ny / 2; i < Ny; i++)
		ky[i] = 2 * Pi * (i - Ny) * 1.0 / dy / Ny;
	for (i = 0; i < Ny; i++)
		ky[i] *= ky[i];

	for (i = 0; i <= Nz / 2 - 1; i++)
		kz[i] = 2 * Pi * i * 1.0 / Nz / dz;
	for (i = Nz / 2; i < Nz; i++)
		kz[i] = 2 * Pi * (i - Nz) * 1.0 / dz / Nz;
	for (i = 0; i < Nz; i++)
		kz[i] *= kz[i];

	for (i = 0; i < Nx; i++)
		for (j = 0; j < Ny; j++)
			for (k = 0; k < Nz; k++)
			{
				ijk = (long)((i * Ny + j) * Nz + k);
				kxyz[ijk] = kx[i] + ky[j] + kz[k];
			}

	/***************Initialize wA, wB******************/
	if (aismatrix == 0)
	{
		fAinit = fA;
		fBinit = fB;
		fCinit = fC;
		init(in, wA, wB, wC);
	}
	else if (aismatrix == 1)
	{
		fAinit = fC;
		fBinit = fB;
		fCinit = fA;
		init(in, wC, wB, wA);
	}

	freeE(wA, wB, wC, phA, phB, phC, eta);

	free(wA);
	free(wB);
	free(wC);
	free(phA);
	free(phB);
	free(phC);
	free(eta);
	free(kxyz);

	return 1;
}

void init(int in, double *wA, double *wB, double *wC)
{
	FILE *fp;
	long ijk;
	int i, j, k;
	double e1, e2, e3, e4, e5, e6;

	if (in == 0)
		initW_Random(wA, wB, wC);
	else if (in == 15)
		initW_csA15(wA, wB, wC);
	else if (in == 233)
		initW_csBCC(wA, wB, wC);
	else if (in == 633)
		initW_csFCC(wA, wB, wC);
	else if (in == 1619)
		initW_csPS(wA, wB, wC);
	else if (in == 3)
		initW_csC(wA, wB, wC);
	else if (in == 12)
		initW_LAM2(wA, wB, wC);
	else if (in == 8)
		initW_csHelix(wA, wB, wC);
	else if (in == 183)
		initW_RC(wA, wB, wC);
	else if (in == 813)
		initW_H1C(wA, wB, wC);
	else if (in == 11619)
		initW_aPS(wA, wB, wC);
	else if (in == 82)
		initW_csHelix2(wA, wB, wC);
	else if (in == 1)
	{
		fp = fopen("in.d", "r");
		for (i = 0; i < Nx; i++)
			for (j = 0; j < Ny; j++)
				for (k = 0; k < Nz; k++)
				{
					fscanf(fp, "%lf %lf %lf %lf %lf %lf", &e1, &e2, &e3, &e4, &e5, &e6);

					ijk = (long)((i * Ny + j) * Nz + k);
					wA[ijk] = e4;
					wB[ijk] = e5;
					wC[ijk] = e6;
				}
		fclose(fp);
	}
	else if (in == 2)
	{
		fp = fopen("in.d", "r");
		for (i = 0; i < Nx; i++)
			for (j = 0; j < Ny; j++)
				for (k = 0; k < Nz; k++)
				{
					fscanf(fp, "%lf %lf %lf %lf %lf %lf", &e1, &e2, &e3, &e4, &e5, &e6);

					ijk = (long)((i * Ny + j) * Nz + k);
					wA[ijk] = hAB * e2 + hAC * e3;
					wB[ijk] = hAB * e1 + hBC * e3;
					wC[ijk] = hAC * e1 + hBC * e2;
				}
		fclose(fp);
	}
}

void initW_Random(double *wA, double *wB, double *wC)
{
	int i, j, k;
	long ijk;

	for (i = 0; i < Nx; i++)
	{
		for (j = 0; j < Ny; j++)
			for (k = 0; k < Nz; k++)
			{
				ijk = (long)((i * Ny + j) * Nz + k);

				wA[ijk] = hAB * fB + hAC * fC + 0.10 * (drand48() - 0.5);
				wB[ijk] = hAB * fA + hBC * fC + 0.10 * (drand48() - 0.5);
				wC[ijk] = hAC * fA + hBC * fB + 0.10 * (drand48() - 0.5);
			}
	}
}

void initW_csA15(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[21], yc[21], zc[21];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csA15.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.0;
	yc[0] = 0.0;
	zc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = 0.0;
	zc[1] = lz;
	xc[2] = 0.0;
	yc[2] = ly;
	zc[2] = 0.0;
	xc[3] = 0.0;
	yc[3] = ly;
	zc[3] = lz;
	xc[4] = lx;
	yc[4] = 0.0;
	zc[4] = lz;
	xc[5] = lx;
	yc[5] = ly;
	zc[5] = 0.0;
	xc[6] = lx;
	yc[6] = ly;
	zc[6] = lz;
	xc[7] = lx;
	yc[7] = 0.0;
	zc[7] = 0.0;
	xc[8] = lx / 2;
	yc[8] = ly / 2;
	zc[8] = lz / 2;

	xc[9] = lx / 2;
	yc[9] = 0.0;
	zc[9] = lz * 0.25;
	xc[10] = lx / 2;
	yc[10] = 0.0;
	zc[10] = lz * 0.75;
	xc[11] = lx / 2;
	yc[11] = ly;
	zc[11] = lz * 0.25;
	xc[12] = lx / 2;
	yc[12] = ly;
	zc[12] = lz * 0.75;
	xc[13] = 0.0;
	yc[13] = ly * 0.25;
	zc[13] = lz / 2;
	xc[14] = 0.0;
	yc[14] = ly * 0.75;
	zc[14] = lz / 2;
	xc[15] = lx;
	yc[15] = ly * 0.25;
	zc[15] = lz / 2;
	xc[16] = lx;
	yc[16] = ly * 0.75;
	zc[16] = lz / 2;
	xc[17] = lx * 0.25;
	yc[17] = ly / 2;
	zc[17] = 0.0;
	xc[18] = lx * 0.75;
	yc[18] = ly / 2;
	zc[18] = 0.0;
	xc[19] = lx * 0.25;
	yc[19] = ly / 2;
	zc[19] = lz;
	xc[20] = lx * 0.75;
	yc[20] = ly / 2;
	zc[20] = lz;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 0;
				for (nc = 0; nc < 9; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij) * pow(4.0, 1.0 / 3);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				for (nc = 9; nc < 13; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij * 2 + yij * yij * 4 + zij * zij * 4;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				for (nc = 13; nc < 17; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij * 4 + yij * yij * 4 + zij * zij * 2;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				for (nc = 17; nc < 21; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij * 4 + yij * yij * 2 + zij * zij * 4;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_csBCC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[9], yc[9], zc[9];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csBCC.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.0;
	yc[0] = 0.0;
	zc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = 0.0;
	zc[1] = lz;
	xc[2] = 0.0;
	yc[2] = ly;
	zc[2] = 0.0;
	xc[3] = 0.0;
	yc[3] = ly;
	zc[3] = lz;
	xc[4] = lx;
	yc[4] = 0.0;
	zc[4] = lz;
	xc[5] = lx;
	yc[5] = ly;
	zc[5] = 0.0;
	xc[6] = lx;
	yc[6] = ly;
	zc[6] = lz;
	xc[7] = lx;
	yc[7] = 0.0;
	zc[7] = 0.0;
	xc[8] = lx / 2;
	yc[8] = ly / 2;
	zc[8] = lz / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 0;
				for (nc = 0; nc < 9; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_csFCC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[15], yc[15], zc[15];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csFCC.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.0;
	yc[0] = 0.0;
	zc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = 0.0;
	zc[1] = lz;
	xc[2] = 0.0;
	yc[2] = ly;
	zc[2] = 0.0;
	xc[3] = 0.0;
	yc[3] = ly;
	zc[3] = lz;
	xc[4] = lx;
	yc[4] = 0.0;
	zc[4] = lz;
	xc[5] = lx;
	yc[5] = ly;
	zc[5] = 0.0;
	xc[6] = lx;
	yc[6] = ly;
	zc[6] = lz;
	xc[7] = lx;
	yc[7] = 0.0;
	zc[7] = 0.0;
	xc[8] = lx / 2;
	yc[8] = ly / 2;
	zc[8] = lz / 2;
	xc[9] = lx / 2;
	yc[9] = ly / 2;
	zc[9] = 0.0;
	xc[10] = lx / 2;
	yc[10] = ly / 2;
	zc[10] = lz;
	xc[11] = 0.0;
	yc[11] = ly / 2;
	zc[11] = lz / 2;
	xc[12] = lx;
	yc[12] = ly / 2;
	zc[12] = lz / 2;
	xc[13] = lx / 2;
	yc[13] = 0.0;
	zc[13] = lz / 2;
	xc[14] = lx / 2;
	yc[14] = ly;
	zc[14] = lz / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 0;
				for (nc = 0; nc < 15; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

/* needs relatively small fA and fC, domain of C will form a framework of Volloni Cells */
void initW_csPS(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[8], yc[8], zc[8];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csPS.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (2 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (2 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.0;
	yc[0] = 0.0;
	zc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = 0.0;
	zc[1] = lz;
	xc[2] = 0.0;
	yc[2] = ly;
	zc[2] = 0.0;
	xc[3] = 0.0;
	yc[3] = ly;
	zc[3] = lz;
	xc[4] = lx;
	yc[4] = 0.0;
	zc[4] = lz;
	xc[5] = lx;
	yc[5] = ly;
	zc[5] = 0.0;
	xc[6] = lx;
	yc[6] = ly;
	zc[6] = lz;
	xc[7] = lx;
	yc[7] = 0.0;
	zc[7] = 0.0;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 0;
				for (nc = 0; nc < 8; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_csC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[5], yc[5];
	double xi, yj, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csCHex.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rb = pow((fAinit + fBinit) / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

	xc[0] = 0.0;
	yc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = ly;
	xc[2] = lx;
	yc[2] = 0.0;
	xc[3] = lx;
	yc[3] = ly;
	xc[4] = lx / 2;
	yc[4] = ly / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				tag = 0;
				for (nc = 0; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_LAM2(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij;
	double xc[2];
	double xi, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_LAM2.dat", "w");

	ra = fAinit / (fA + fB + fC) * lx / 2;
	rb = (fAinit + fBinit) / (fA + fB + fC) * lx / 2;

	xc[0] = 0.0;
	xc[1] = lx;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			for (k = 0; k < Nz; k++)
			{
				tag = 0;
				for (nc = 0; nc < 2; nc++)
				{
					xij = xi - xc[nc];
					rij = sqrt(xij * xij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}
				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_csHelix(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[10], yc[10];
	double xi, yj, zk, theta, rij, rb, ra, r;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csHelix.dat", "w");

	r = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);
	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = lx / 2;
	yc[0] = ly / 2;
	xc[1] = 0.0;
	yc[1] = ly;
	xc[2] = 0.0;
	yc[2] = 0.0;
	xc[3] = lx;
	yc[3] = 0.0;
	xc[4] = lx;
	yc[4] = ly;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				theta = zk / lz * 2 * Pi;

				tag = 0;
				for (nc = 5; nc < 10; nc++)
				{
					xc[nc] = xc[nc - 5] + r * cos(theta + Pi / 2);
					yc[nc] = yc[nc - 5] + r * sin(theta + Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}

				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_RC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[5], yc[5];
	double xi, yj, zk, rij, rb, ra, rc;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_RC.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);
	rb = pow((fAinit + fBinit) / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);
	rc = pow((fAinit - fBinit) / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);
	xc[0] = 0.0;
	yc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = ly;
	xc[2] = lx;
	yc[2] = 0.0;
	xc[3] = lx;
	yc[3] = ly;
	xc[4] = lx / 2;
	yc[4] = ly / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 0;

				for (nc = 0; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < ra)
					{
						tag = 1;
					}
					if (nc == 4)
					{
						if ((zk < (rb - rc) / 2) || (zk > lz - (rb - rc) / 2))
						{
							if ((rij < rb) && (rij > rc))
							{
								tag = 2;
							}
						}
					}
					else
					{
						if ((zk > lz / 2 - (rb - rc) / 2) && (zk < lz / 2 + (rb - rc) / 2))
						{
							if ((rij < rb) && (rij > rc))
							{
								tag = 2;
							}
						}
					}
				}

				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_H1C(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[10], yc[10];
	double xi, yj, zk, rij, rb, ra, rc;
	double phat, phbt, phct;
	double theta;
	FILE *fp;
	fp = fopen("init_H1C.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);
	rb = pow(fBinit / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);

	xc[0] = 0.0;
	yc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = ly;
	xc[2] = lx;
	yc[2] = 0.0;
	xc[3] = lx;
	yc[3] = ly;
	xc[4] = lx / 2;
	yc[4] = ly / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				theta = zk / lz * 2 * Pi;
				tag = 0;

				for (nc = 0; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < ra)
						tag = 1;
				}

				for (nc = 5; nc < 10; nc++)
				{
					xc[nc] = xc[nc - 5] + ra * cos(theta + Pi / 2);
					yc[nc] = yc[nc - 5] + ra * sin(theta + Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_aPS(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[9], yc[9], zc[9];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aPS.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (2 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (2 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.0;
	yc[0] = 0.0;
	zc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = 0.0;
	zc[1] = lz;
	xc[2] = 0.0;
	yc[2] = ly;
	zc[2] = 0.0;
	xc[3] = 0.0;
	yc[3] = ly;
	zc[3] = lz;
	xc[4] = lx;
	yc[4] = 0.0;
	zc[4] = lz;
	xc[5] = lx;
	yc[5] = ly;
	zc[5] = 0.0;
	xc[6] = lx;
	yc[6] = ly;
	zc[6] = lz;
	xc[7] = lx;
	yc[7] = 0.0;
	zc[7] = 0.0;
	xc[8] = lx / 2;
	yc[8] = ly / 2;
	zc[8] = lz / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 0;
				for (nc = 0; nc < 9; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if ((rij < rb) && (nc < 8))
						tag = 1;
					else if ((rij < ra) && (nc == 8))
						tag = 2;
				}
				phat = 0.0;
				phbt = 1.0;
				phct = 0.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 0.0;
					phct = 1.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

void initW_csHelix2(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[15], yc[15];
	double xi, yj, zk, theta, rij, rb, ra, r;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csHelix2.dat", "w");

	r = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);
	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = lx / 2;
	yc[0] = ly / 2;
	xc[1] = 0.0;
	yc[1] = ly;
	xc[2] = 0.0;
	yc[2] = 0.0;
	xc[3] = lx;
	yc[3] = 0.0;
	xc[4] = lx;
	yc[4] = ly;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				theta = zk / lz * 2 * Pi;

				tag = 0;
				for (nc = 5; nc < 15; nc++)
				{
					if (nc < 10)
					{
						xc[nc] = xc[nc - 5] + r * cos(theta + Pi / 2);
						yc[nc] = yc[nc - 5] + r * sin(theta + Pi / 2);
					}
					else
					{
						xc[nc] = xc[nc - 10] + r / 2 * cos(theta - Pi / 2);
						yc[nc] = yc[nc - 10] + r / 2 * sin(theta - Pi / 2);
					}

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rb)
					{
						tag = 1;
						if (rij < ra)
						{
							tag = 2;
						}
					}
				}

				phat = 0.0;
				phbt = 0.0;
				phct = 1.0;

				if (tag == 1)
				{
					phat = 0.0;
					phbt = 1.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				ijk = (long)((i * Ny + j) * Nz + k);
				wA[ijk] = hAB * phbt + hAC * phct + 0.040 * (drand48() - 0.5);
				wB[ijk] = hAB * phat + hBC * phct + 0.040 * (drand48() - 0.5);
				wC[ijk] = hAC * phat + hBC * phbt + 0.040 * (drand48() - 0.5);
				if (aismatrix == 0)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phat, phbt, phct, wA[ijk], wB[ijk], wC[ijk]);
				else if (aismatrix == 1)
					fprintf(fp, "%lf %lf %lf %lf %lf %lf\n", phct, phbt, phat, wC[ijk], wB[ijk], wA[ijk]);
			}
		}
	}
	fclose(fp);
}

//********************Output configuration******************************

void write_ph(double *phA, double *phB, double *phC, double *wA, double *wB, double *wC)
{
	int i, j, k;
	long ijk;
	FILE *fp = fopen(phname, "w");
	//	FILE *fpp=fopen("result.dat","w");
	//	fprintf(fp,"Nx=%d, Ny=%d, Nz=%d\n",Nx,Ny,Nz);
	//	fprintf(fp,"dx=%lf, dy=%lf, dz=%lf\n",dx,dy,dz);

	for (i = 0; i < Nx; i++)
	{
		for (j = 0; j < Ny; j++)
		{
			for (k = 0; k < Nz; k++)
			{
				ijk = (long)((i * Ny + j) * Nz + k);
				fprintf(fp, "%lf %lf %lf %lf %lf %lf\n",
						phA[ijk], phB[ijk], phC[ijk], wA[ijk], wB[ijk], wC[ijk]);
			}
			fprintf(fp, "\n");
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

//*************************************main loop****************************************

double freeE(double *wA, double *wB, double *wC, double *phA, double *phB, double *phC, double *eta)
{
	int i, j, k, iter, maxIter;
	long ijk;
	double freeEnergy, freeOld, qC;
	double freeW, freeAB, freeS, freeDiff;
	double Sm1, Sm2, beta, psum, fpsum, *psuC;
	double *waDiff, *wbDiff, *wcDiff, inCompMax, wa0, wb0, wc0;
	double *del, *outs, *wAnew, *wBnew, *wCnew, err;
	int N_rec;
	FILE *fp;

	psuC = (double *)malloc(sizeof(double) * NxNyNz);
	waDiff = (double *)malloc(sizeof(double) * NxNyNz);
	wbDiff = (double *)malloc(sizeof(double) * NxNyNz);
	wcDiff = (double *)malloc(sizeof(double) * NxNyNz);
	wAnew = (double *)malloc(sizeof(double) * NxNyNz);
	wBnew = (double *)malloc(sizeof(double) * NxNyNz);
	wCnew = (double *)malloc(sizeof(double) * NxNyNz);
	del = (double *)malloc(sizeof(double) * N_hist * 3 * NxNyNz);
	outs = (double *)malloc(sizeof(double) * N_hist * 3 * NxNyNz);

	Sm1 = 1.0e-6;
	Sm2 = 0.1e-9;
	maxIter = MaxIT;
	beta = 1.0;

	iter = 0;

	freeEnergy = 0.0;
	// FILE *fp1=fopen("result.dat","w");

	do
	{
		iter = iter + 1;

		qC = getConc(phA, phB, phC, 1.0, wA, wB, wC);

		freeW = 0.0;
		freeAB = 0.0;
		freeS = 0.0;

		inCompMax = 0.0;

		for (ijk = 0; ijk < NxNyNz; ijk++)
		{
			eta[ijk] = (wA[ijk] + wB[ijk] + wC[ijk] - hAB * (1.0 - phC[ijk]) - hBC * (1.0 - phA[ijk]) - hAC * (1.0 - phB[ijk])) / 3.0;
			psum = 1.0 - phA[ijk] - phB[ijk] - phC[ijk];
			psuC[ijk] = psum;
			fpsum = fabs(psum);
			if (fpsum > inCompMax)
				inCompMax = fpsum;
			wAnew[ijk] = hAB * phB[ijk] + hAC * phC[ijk] + eta[ijk];
			wBnew[ijk] = hAB * phA[ijk] + hBC * phC[ijk] + eta[ijk];
			wCnew[ijk] = hAC * phA[ijk] + hBC * phB[ijk] + eta[ijk];
			waDiff[ijk] = wAnew[ijk] - wA[ijk];
			wbDiff[ijk] = wBnew[ijk] - wB[ijk];
			wcDiff[ijk] = wCnew[ijk] - wC[ijk];
			waDiff[ijk] -= wcmp * psum;
			wbDiff[ijk] -= wcmp * psum;
			wcDiff[ijk] -= wcmp * psum;

			freeAB += (hAB * phA[ijk] * phB[ijk] + hAC * phA[ijk] * phC[ijk] + hBC * phB[ijk] * phC[ijk]);
			freeW -= (wA[ijk] * phA[ijk] + wB[ijk] * phB[ijk] + wC[ijk] * phC[ijk] + eta[ijk] * psum);
		}

		freeAB /= NxNyNz;
		freeW /= NxNyNz;

		freeS = -log(qC);

		freeOld = freeEnergy;
		freeEnergy = freeAB + freeW + freeS;

		//judge the error
		err = error_cal(waDiff, wbDiff, wcDiff, wA, wB, wC);

		//update the history fields, and zero is new fields
		update_flds_hist(waDiff, wbDiff, wcDiff, wAnew, wBnew, wCnew, del, outs);

		//if achieved some level, anderson-mixing, else simple-mixing
		if ((err > errMax || iter < 3 || andmix == 0) && (andmix != 2 || iter < andmixsteps))
		{
			for (ijk = 0; ijk < NxNyNz; ijk++)
			{
				wA[ijk] += wopt * waDiff[ijk];
				wB[ijk] += wopt * wbDiff[ijk];
				wC[ijk] += wopt * wcDiff[ijk];
			}
		}
		else
		{
			printf("iter  %4d  err  %.4f /***** enter Anderson mixing *****/\n", iter, err);
			N_rec = (iter - 1) < N_hist ? (iter - 1) : N_hist;
			Anderson_mixing(del, outs, N_rec, wA, wB, wC);
		}

		//**** print out the free energy and error results ****

		if (iter == 1 || iter % 20 == 0 || iter >= maxIter)
		{
			if (iter == 1)
				fp = fopen("printout.txt", "w");
			else
				fp = fopen("printout.txt", "a");
			fprintf(fp, "%d\n", iter);
			fprintf(fp, "%10.8e, %10.8e, %10.8e, %10.8e, %e\n", freeEnergy, freeAB, freeW, freeS, inCompMax);
			fclose(fp);
		}
		printf(" %5d : %.8e, %.8e, %.8e\n", iter, freeEnergy, inCompMax, err);
		freeDiff = fabs(freeEnergy - freeOld);

		if (iter == 1 || iter % 1000 == 0)
			write_ph(phA, phB, phC, wA, wB, wC);
	} while (iter < maxIter && (inCompMax > Sm1 || freeDiff > Sm2));

	fp = fopen("result.dat", "w");
	fprintf(fp, "%d\n", iter);
	fprintf(fp, "%10.8e, %10.8e, %10.8e, %e\n", freeEnergy, freeAB, freeW, freeS, inCompMax);
	fclose(fp);
	//fclose(fp1);
	write_ph(phA, phB, phC, wA, wB, wC);

	free(psuC);
	free(waDiff);
	free(wbDiff);
	free(wcDiff);
	free(wAnew);
	free(wBnew);
	free(wCnew);
	free(del);
	free(outs);

	return freeDiff;
}

double getConc(phlA, phlB, phlC, phs0, wA, wB, wC) double *phlA, *phlB, *phlC, phs0;
double *wA, *wB, *wC;
{
	int i, j, k, iz;
	long ijk, ijkiz;
	double *qA, *qcA, *qC, *qcC, *qB, *qcB;
	double ql, ffl, fflA, fflB, fflC, *qInt, qtmp;

	qA = (double *)malloc(sizeof(double) * NxNyNz * (NsA + 1));
	qcA = (double *)malloc(sizeof(double) * NxNyNz * (NsA + 1));
	qB = (double *)malloc(sizeof(double) * NxNyNz * (NsB + 1));
	qcB = (double *)malloc(sizeof(double) * NxNyNz * (NsB + 1));
	qC = (double *)malloc(sizeof(double) * NxNyNz * (NsC + 1));
	qcC = (double *)malloc(sizeof(double) * NxNyNz * (NsC + 1));
	qInt = (double *)malloc(sizeof(double) * NxNyNz);

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		qInt[ijk] = 1.0;
	}

	sovDifFft(qA, wA, qInt, fA, NsA, 1, epA);
	sovDifFft(qcC, wC, qInt, fC, NsC, -1, epC);

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		qtmp = qcC[ijk * (NsC + 1)];
		qInt[ijk] = qcC[ijk * (NsC + 1)];
		qInt[ijk] *= qtmp;
	}

	sovDifFft(qcB, wB, qInt, fB, NsB, -1, epB);

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		qtmp = qcB[ijk * (NsB + 1)];
		qInt[ijk] = qcB[ijk * (NsB + 1)];
		qInt[ijk] *= qtmp;
	}

	sovDifFft(qcA, wA, qInt, fA, NsA, -1, epA);

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		qtmp = qcB[ijk * (NsB + 1)];
		qInt[ijk] = qA[ijk * (NsA + 1) + NsA];
		qInt[ijk] *= qtmp;
	}
	sovDifFft(qB, wB, qInt, fB, NsB, 1, epB);

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		qtmp = qcC[ijk * (NsC + 1)];
		qInt[ijk] = qB[ijk * (NsB + 1) + NsB];
		qInt[ijk] *= qtmp;
	}

	sovDifFft(qC, wC, qInt, fC, NsC, 1, epC);

	ql = 0.0;
	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		ql += qcA[ijk * (NsA + 1)];
	}

	ql /= NxNyNz;

	ffl = phs0 / ql;
	fflA = ds0 * ffl;
	fflB = ds0 * ffl * 2;
	fflC = ds0 * ffl * 4;

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		phlA[ijk] = 0.0;
		phlB[ijk] = 0.0;
		phlC[ijk] = 0.0;

		ZDIMM = NsA + 1;
		for (iz = 0; iz <= NsA; iz++)
		{
			ijkiz = ijk * ZDIMM + iz;
			if (iz == 0 || iz == NsA)
				phlA[ijk] += (0.50 * qA[ijkiz] * qcA[ijkiz]);
			else
				phlA[ijk] += (qA[ijkiz] * qcA[ijkiz]);
		}

		ZDIMM = NsB + 1;
		for (iz = 0; iz <= NsB; iz++)
		{
			ijkiz = ijk * ZDIMM + iz;
			if (iz == 0 || iz == NsB)
				phlB[ijk] += (0.50 * qB[ijkiz] * qcB[ijkiz]);
			else
				phlB[ijk] += (qB[ijkiz] * qcB[ijkiz]);
		}

		ZDIMM = NsC + 1;
		for (iz = 0; iz <= NsC; iz++)
		{
			ijkiz = ijk * ZDIMM + iz;
			if (iz == 0 || iz == NsC)
				phlC[ijk] += (0.50 * qC[ijkiz] * qcC[ijkiz]);
			else
				phlC[ijk] += (qC[ijkiz] * qcC[ijkiz]);
		}

		phlA[ijk] *= fflA;
		phlB[ijk] *= fflB;
		phlC[ijk] *= fflC;
	}
	free(qA);
	free(qC);
	free(qcA);
	free(qcC);
	free(qB);
	free(qcB);
	free(qInt);

	return ql;
}

void sovDifFft(double *g, double *w, double *qInt, double z, int ns, int sign, double epk)
{
	int i, j, k, iz;
	unsigned long ijk, ijkr;
	double dzc, *wdz;
	double *kxyzdz, dzc2;
	double *in;
	fftw_complex *out;
	fftw_plan p_forward, p_backward;

	wdz = (double *)malloc(sizeof(double) * NxNyNz);
	kxyzdz = (double *)malloc(sizeof(double) * NxNyNz);
	in = (double *)malloc(sizeof(double) * NxNyNz);

	out = (fftw_complex *)malloc(sizeof(fftw_complex) * NxNyNz1);
	dzc = z / ns;
	dzc2 = 0.50 * dzc;
	ZDIMM = ns + 1;
	for (i = 0; i < Nx; i++)
		for (j = 0; j < Ny; j++)
			for (k = 0; k < Nz; k++)
			{
				ijk = (i * Ny + j) * Nz + k;
				kxyzdz[ijk] = exp(-dzc * kxyz[ijk] * epk);
				wdz[ijk] = exp(-w[ijk] * dzc2);
			}
	p_forward = fftw_plan_dft_r2c_3d(Nx, Ny, Nz, in, out, FFTW_ESTIMATE);
	p_backward = fftw_plan_dft_c2r_3d(Nx, Ny, Nz, out, in, FFTW_ESTIMATE);
	if (sign == 1)
	{
		for (ijk = 0; ijk < NxNyNz; ijk++)
		{
			g[ijk * ZDIMM] = qInt[ijk];
		}

		for (iz = 1; iz <= ns; iz++)
		{
			for (ijk = 0; ijk < NxNyNz; ijk++)
			{
				in[ijk] = g[ijk * ZDIMM + iz - 1] * wdz[ijk];
			}

			fftw_execute(p_forward);

			for (i = 0; i < Nx; i++)
				for (j = 0; j < Ny; j++)
					for (k = 0; k < Nzh1; k++)
					{
						ijk = (i * Ny + j) * Nzh1 + k;
						ijkr = (i * Ny + j) * Nz + k;
						out[ijk][0] *= kxyzdz[ijkr]; //out[].re or .im for fftw2
						out[ijk][1] *= kxyzdz[ijkr]; //out[][0] or [1] for fftw3
					}

			fftw_execute(p_backward);

			for (ijk = 0; ijk < NxNyNz; ijk++)
			{
				g[ijk * ZDIMM + iz] = in[ijk] * wdz[ijk] / NxNyNz;
			}
		}
	}
	else
	{

		for (ijk = 0; ijk < NxNyNz; ijk++)
		{
			g[ijk * ZDIMM + ns] = qInt[ijk];
		}

		for (iz = ns - 1; iz >= 0; iz--)
		{
			for (ijk = 0; ijk < NxNyNz; ijk++)
			{
				in[ijk] = g[ijk * ZDIMM + iz + 1] * wdz[ijk];
			}

			fftw_execute(p_forward);

			for (i = 0; i < Nx; i++)
				for (j = 0; j < Ny; j++)
					for (k = 0; k < Nzh1; k++)
					{
						ijk = (i * Ny + j) * Nzh1 + k;
						ijkr = (i * Ny + j) * Nz + k;
						out[ijk][0] *= kxyzdz[ijkr];
						out[ijk][1] *= kxyzdz[ijkr];
					}

			fftw_execute(p_backward);

			for (ijk = 0; ijk < NxNyNz; ijk++)
			{
				//ijk=(i*Ny+j)*Nz2+k;
				//ijkr=(i*Ny+j)*Nz+k;
				g[ijk * ZDIMM + iz] = in[ijk] * wdz[ijk] / NxNyNz;
			}
		}
	}
	fftw_destroy_plan(p_forward);
	fftw_destroy_plan(p_backward);
	free(wdz);
	free(kxyzdz);
	free(in);
	free(out);
}

double error_cal(double *waDiffs, double *wbDiffs, double *wcDiffs, double *wAs, double *wBs, double *wCs)
{
	double err_dif, err_w, err;
	int ijk;

	err = 0.0;
	err_dif = 0.0;
	err_w = 0.0;
	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		err_dif += pow(waDiffs[ijk], 2) + pow(wbDiffs[ijk], 2) + pow(wcDiffs[ijk], 2);
		err_w += pow(wAs[ijk], 2) + pow(wBs[ijk], 2) + pow(wCs[ijk], 2);
	}
	err = err_dif / err_w;
	err = sqrt(err);

	return err;
}

void update_flds_hist(double *waDiff, double *wbDiff, double *wcDiff, double *wAnew, double *wBnew, double *wCnew, double *del, double *outs)
{
	int ijk, j;

	for (j = N_hist - 1; j > 0; j--)
	{
		for (ijk = 0; ijk < NxNyNz; ijk++)
		{
			Del(0, ijk, j) = Del(0, ijk, j - 1);
			Del(1, ijk, j) = Del(1, ijk, j - 1);
			Del(2, ijk, j) = Del(2, ijk, j - 1);
			Outs(0, ijk, j) = Outs(0, ijk, j - 1);
			Outs(1, ijk, j) = Outs(1, ijk, j - 1);
			Outs(2, ijk, j) = Outs(2, ijk, j - 1);
		}
	}

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		Del(0, ijk, 0) = waDiff[ijk];
		Del(1, ijk, 0) = wbDiff[ijk];
		Del(2, ijk, 0) = wcDiff[ijk];
		Outs(0, ijk, 0) = wAnew[ijk];
		Outs(1, ijk, 0) = wBnew[ijk];
		Outs(2, ijk, 0) = wCnew[ijk];
	}
}

/*********************************************************************/
/*
  Anderson mixing [O(Nx)]

  CHECKED
*/

void Anderson_mixing(double *del, double *outs, int N_rec, double *wA, double *wB, double *wC)
{
	int i, k, ijk;
	int n, m;
	double *up, *vp, *ap;
	int s;

	gsl_matrix_view uGnu;
	gsl_vector_view vGnu, aGnu;
	gsl_permutation *p;

	up = (double *)malloc(sizeof(double) * (N_rec - 1) * (N_rec - 1));
	vp = (double *)malloc(sizeof(double) * (N_rec - 1));
	ap = (double *)malloc(sizeof(double) * (N_rec - 1));

	/* 
	     Calculate the U-matrix and the V-vector 
     
		Follow Shuang, and add the A and B components together.
  	*/

	for (n = 1; n < N_rec; n++)
	{
		V(n) = 0.0;

		for (ijk = 0; ijk < NxNyNz; ijk++)
		{
			V(n) += (Del(0, ijk, 0) - Del(0, ijk, n)) * Del(0, ijk, 0);
			V(n) += (Del(1, ijk, 0) - Del(1, ijk, n)) * Del(1, ijk, 0);
			V(n) += (Del(2, ijk, 0) - Del(2, ijk, n)) * Del(2, ijk, 0);
		}

		for (m = n; m < N_rec; m++)
		{
			U(n, m) = 0.0;
			for (ijk = 0; ijk < NxNyNz; ijk++)
			{
				U(n, m) += (Del(0, ijk, 0) - Del(0, ijk, n)) * (Del(0, ijk, 0) - Del(0, ijk, m));
				U(n, m) += (Del(1, ijk, 0) - Del(1, ijk, n)) * (Del(1, ijk, 0) - Del(1, ijk, m));
				U(n, m) += (Del(2, ijk, 0) - Del(2, ijk, n)) * (Del(2, ijk, 0) - Del(2, ijk, m));
			}
			U(m, n) = U(n, m);
		}
	}

	/* Calculate A - uses GNU LU decomposition for U A = V */

	uGnu = gsl_matrix_view_array(up, N_rec - 1, N_rec - 1);
	vGnu = gsl_vector_view_array(vp, N_rec - 1);
	aGnu = gsl_vector_view_array(ap, N_rec - 1);

	p = gsl_permutation_alloc(N_rec - 1);

	gsl_linalg_LU_decomp(&uGnu.matrix, p, &s);

	gsl_linalg_LU_solve(&uGnu.matrix, p, &vGnu.vector, &aGnu.vector);

	gsl_permutation_free(p);

	/* Update omega */

	for (ijk = 0; ijk < NxNyNz; ijk++)
	{
		wA[ijk] = Outs(0, ijk, 0);
		wB[ijk] = Outs(1, ijk, 0);
		wC[ijk] = Outs(2, ijk, 0);

		for (n = 1; n < N_rec; n++)
		{
			wA[ijk] += A(n) * (Outs(0, ijk, n) - Outs(0, ijk, 0));
			wB[ijk] += A(n) * (Outs(1, ijk, n) - Outs(1, ijk, 0));
			wC[ijk] += A(n) * (Outs(2, ijk, n) - Outs(2, ijk, 0));
		}
	}

	free(ap);
	free(vp);
	free(up);
}
