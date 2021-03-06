/////////AB2C4 dendritic triblock copolymer //////
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
void initW_csHelix(double *wA, double *wB, double *wC);
void initW_csHelix2(double *wA, double *wB, double *wC);
void initW_csG(double *wA, double *wB, double *wC);
void initW_aG(double *wA, double *wB, double *wC);
void initW_csSigma(double *wA, double *wB, double *wC);
void initW_csSigma_z(double *wA, double *wB, double *wC);
void initW_LAM2(double *wA, double *wB, double *wC);
void initW_RC(double *wA, double *wB, double *wC);
void initW_H1C(double *wA, double *wB, double *wC);
void initW_H2C(double *wA, double *wB, double *wC);
void initW_H3C(double *wA, double *wB, double *wC);
void initW_H4C(double *wA, double *wB, double *wC);
void initW_aPS(double *wA, double *wB, double *wC);
void initW_aBCC(double *wA, double *wB, double *wC);
void initW_aFCC(double *wA, double *wB, double *wC);
void initW_aC(double *wA, double *wB, double *wC);
void initW_aA15(double *wA, double *wB, double *wC);
void initW_aC42(double *wA, double *wB, double *wC);
void initW_aC62(double *wA, double *wB, double *wC);
void initW_aC63(double *wA, double *wB, double *wC);
void initW_aC3(double *wA, double *wB, double *wC);
void initW_aZ(double *wA, double *wB, double *wC);
void initW_aS30(double *wA, double *wB, double *wC);
void initW_aS24(double *wA, double *wB, double *wC);
void initW_csPL(double *wA, double *wB, double *wC);
void initW_csPL_z(double *wA, double *wB, double *wC);
void initW_csC14(double *wA, double *wB, double *wC);
void initW_csC14_z(double *wA, double *wB, double *wC);
void initW_csC15(double *wA, double *wB, double *wC);
void initW_csC36(double *wA, double *wB, double *wC);
void initW_csZ(double *wA, double *wB, double *wC);
void initW_aLC(double *wA, double *wB, double *wC);
void initW_aCS63(double *wA, double *wB, double *wC);
void initW_aSC63(double *wA, double *wB, double *wC);

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

	if (lylx != 0)
		ly = lx * sqrt(lylx);
	if (lzlx != 0)
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
	else if (in == 823)
		initW_H2C(wA, wB, wC);
	else if (in == 833)
		initW_H3C(wA, wB, wC);
	else if (in == 843)
		initW_H4C(wA, wB, wC);
	else if (in == 1233)
		initW_aBCC(wA, wB, wC);
	else if (in == 1633)
		initW_aFCC(wA, wB, wC);
	else if (in == 13)
		initW_aC(wA, wB, wC);
	else if (in == 115)
		initW_aA15(wA, wB, wC);
	else if (in == 1342)
		initW_aC42(wA, wB, wC);
	else if (in == 1362)
		initW_aC62(wA, wB, wC);
	else if (in == 1363)
		initW_aC63(wA, wB, wC);
	else if (in == 133)
		initW_aC3(wA, wB, wC);
	else if (in == 11619)
		initW_aPS(wA, wB, wC);
	else if (in == 82)
		initW_csHelix2(wA, wB, wC);
	else if (in == 17)
		initW_aG(wA, wB, wC);
	else if (in == 7)
		initW_csG(wA, wB, wC);
	else if (in == 6)
		initW_csSigma(wA, wB, wC);
	else if (in == 626)
		initW_csSigma_z(wA, wB, wC);
	else if (in == 1612)
		initW_csPL(wA, wB, wC);
	else if (in == 161226)
		initW_csPL_z(wA, wB, wC);
	else if (in == 314)
		initW_csC14(wA, wB, wC);
	else if (in == 31426)
		initW_csC14_z(wA, wB, wC);
	else if (in == 315)
		initW_csC15(wA, wB, wC);
	else if (in == 336)
		initW_csC36(wA, wB, wC);
	else if (in == 26)
		initW_csZ(wA, wB, wC);
	else if (in == 126)
		initW_aZ(wA, wB, wC);
	else if (in == 11930)
		initW_aS30(wA, wB, wC);
	else if (in == 11924)
		initW_aS24(wA, wB, wC);
	else if (in == 1123)
		initW_aLC(wA, wB, wC);
	else if (in == 131963)
		initW_aCS63(wA, wB, wC);
	else if (in == 119363)
		initW_aSC63(wA, wB, wC);
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

void initW_aA15(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[21], yc[21], zc[21];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aA15.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);

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
				tag = 1;
				for (nc = 0; nc < 9; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij) * pow(4.0, 1.0 / 3);
					if (rij < ra)
					{
						tag = 2;
					}
				}
				for (nc = 9; nc < 13; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij * 2 + yij * yij * 4 + zij * zij * 4;
					rij = sqrt(rij);
					if (rij < rc)
					{
						tag = 0;
					}
				}
				for (nc = 13; nc < 17; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij * 4 + yij * yij * 4 + zij * zij * 2;
					rij = sqrt(rij);
					if (rij < rc)
					{
						tag = 0;
					}
				}
				for (nc = 17; nc < 21; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij * 4 + yij * yij * 2 + zij * zij * 4;
					rij = sqrt(rij);
					if (rij < rc)
					{
						tag = 0;
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

void initW_csSigma(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[47], yc[47], zc[47];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csSigma.dat", "w");

	ra = 0.5 * pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = 0.5 * pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.0 * lx;
	yc[0] = 0.0 * ly;
	zc[0] = 0.0 * lz;
	xc[30] = 1.0 * lx;
	yc[30] = 0.0 * ly;
	zc[30] = 0.0 * lz;
	xc[31] = 0.0 * lx;
	yc[31] = 1.0 * ly;
	zc[31] = 0.0 * lz;
	xc[32] = 1.0 * lx;
	yc[32] = 1.0 * ly;
	zc[32] = 0.0 * lz;
	xc[33] = 0.0 * lx;
	yc[33] = 0.0 * ly;
	zc[33] = 1.0 * lz;
	xc[34] = 1.0 * lx;
	yc[34] = 0.0 * ly;
	zc[34] = 1.0 * lz;
	xc[35] = 0.0 * lx;
	yc[35] = 1.0 * ly;
	zc[35] = 1.0 * lz;
	xc[36] = 1.0 * lx;
	yc[36] = 1.0 * ly;
	zc[36] = 1.0 * lz;
	xc[1] = 0.5 * lx;
	yc[1] = 0.5 * ly;
	zc[1] = 0.5 * lz;
	xc[2] = 0.31726 * lx;
	yc[2] = 0.68274 * ly;
	zc[2] = 0.24834 * lz;
	xc[3] = 0.68274 * lx;
	yc[3] = 0.31726 * ly;
	zc[3] = 0.24834 * lz;
	xc[4] = 0.18274 * lx;
	yc[4] = 0.18274 * ly;
	zc[4] = 0.25166 * lz;
	xc[5] = 0.81726 * lx;
	yc[5] = 0.81726 * ly;
	zc[5] = 0.25166 * lz;
	xc[6] = 0.18274 * lx;
	yc[6] = 0.18274 * ly;
	zc[6] = 0.74834 * lz;
	xc[7] = 0.81726 * lx;
	yc[7] = 0.81726 * ly;
	zc[7] = 0.74834 * lz;
	xc[8] = 0.31726 * lx;
	yc[8] = 0.68274 * ly;
	zc[8] = 0.75166 * lz;
	xc[9] = 0.68274 * lx;
	yc[9] = 0.31726 * ly;
	zc[9] = 0.75166 * lz;
	xc[10] = 0.39875 * lx;
	yc[10] = 0.39875 * ly;
	zc[10] = 0.0 * lz;
	xc[37] = 0.39875 * lx;
	yc[37] = 0.39875 * ly;
	zc[37] = 1.0 * lz;
	xc[11] = 0.60125 * lx;
	yc[11] = 0.60125 * ly;
	zc[11] = 0.0 * lz;
	xc[38] = 0.60125 * lx;
	yc[38] = 0.60125 * ly;
	zc[38] = 1.0 * lz;
	xc[12] = 0.89875 * lx;
	yc[12] = 0.10125 * ly;
	zc[12] = 0.5 * lz;
	xc[13] = 0.10125 * lx;
	yc[13] = 0.89875 * ly;
	zc[13] = 0.5 * lz;
	xc[14] = 0.73921 * lx;
	yc[14] = 0.06608 * ly;
	zc[14] = 0.0 * lz;
	xc[15] = 0.93392 * lx;
	yc[15] = 0.26079 * ly;
	zc[15] = 0.0 * lz;
	xc[16] = 0.06608 * lx;
	yc[16] = 0.73921 * ly;
	zc[16] = 0.0 * lz;
	xc[17] = 0.26079 * lx;
	yc[17] = 0.93392 * ly;
	zc[17] = 0.0 * lz;
	xc[39] = 0.73921 * lx;
	yc[39] = 0.06608 * ly;
	zc[39] = 1.0 * lz;
	xc[40] = 0.93392 * lx;
	yc[40] = 0.26079 * ly;
	zc[40] = 1.0 * lz;
	xc[41] = 0.06608 * lx;
	yc[41] = 0.73921 * ly;
	zc[41] = 1.0 * lz;
	xc[42] = 0.26079 * lx;
	yc[42] = 0.93392 * ly;
	zc[42] = 1.0 * lz;
	xc[18] = 0.43392 * lx;
	yc[18] = 0.23921 * ly;
	zc[18] = 0.5 * lz;
	xc[19] = 0.23921 * lx;
	yc[19] = 0.43392 * ly;
	zc[19] = 0.5 * lz;
	xc[20] = 0.76079 * lx;
	yc[20] = 0.56608 * ly;
	zc[20] = 0.5 * lz;
	xc[21] = 0.56608 * lx;
	yc[21] = 0.76079 * ly;
	zc[21] = 0.5 * lz;
	xc[22] = 0.13131 * lx;
	yc[22] = 0.46351 * ly;
	zc[22] = 0.0 * lz;
	xc[23] = 0.46351 * lx;
	yc[23] = 0.13131 * ly;
	zc[23] = 0.0 * lz;
	xc[24] = 0.53649 * lx;
	yc[24] = 0.86869 * ly;
	zc[24] = 0.0 * lz;
	xc[25] = 0.86869 * lx;
	yc[25] = 0.53649 * ly;
	zc[25] = 0.0 * lz;
	xc[43] = 0.13131 * lx;
	yc[43] = 0.46351 * ly;
	zc[43] = 1.0 * lz;
	xc[44] = 0.46351 * lx;
	yc[44] = 0.13131 * ly;
	zc[44] = 1.0 * lz;
	xc[45] = 0.53649 * lx;
	yc[45] = 0.86869 * ly;
	zc[45] = 1.0 * lz;
	xc[46] = 0.86869 * lx;
	yc[46] = 0.53649 * ly;
	zc[46] = 1.0 * lz;
	xc[26] = 0.03649 * lx;
	yc[26] = 0.63131 * ly;
	zc[26] = 0.5 * lz;
	xc[27] = 0.36869 * lx;
	yc[27] = 0.96351 * ly;
	zc[27] = 0.5 * lz;
	xc[28] = 0.63131 * lx;
	yc[28] = 0.03649 * ly;
	zc[28] = 0.5 * lz;
	xc[29] = 0.96351 * lx;
	yc[29] = 0.36869 * ly;
	zc[29] = 0.5 * lz;

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
				for (nc = 0; nc < 47; nc++)
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

void initW_csZ(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[16], yc[16], zc[16];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csZ.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0 * lx;
	yc[0] = 0 * ly;
	zc[0] = 0 * lz;
	xc[1] = 0 * lx;
	yc[1] = 0 * ly;
	zc[1] = 1 * lz;
	xc[2] = 1 * lx;
	yc[2] = 0 * ly;
	zc[2] = 0 * lz;
	xc[3] = 1 * lx;
	yc[3] = 0 * ly;
	zc[3] = 1 * lz;
	xc[4] = 0 * lx;
	yc[4] = 1 * ly;
	zc[4] = 0 * lz;
	xc[5] = 0 * lx;
	yc[5] = 1 * ly;
	zc[5] = 1 * lz;
	xc[6] = 1 * lx;
	yc[6] = 1 * ly;
	zc[6] = 0 * lz;
	xc[7] = 1 * lx;
	yc[7] = 1 * ly;
	zc[7] = 1 * lz;
	xc[8] = 0.5 * lx;
	yc[8] = 0.5 * ly;
	zc[8] = 0 * lz;
	xc[9] = 0.5 * lx;
	yc[9] = 0.5 * ly;
	zc[9] = 1 * lz;
	xc[10] = 0 * lx;
	yc[10] = 0.6667 * ly;
	zc[10] = 0.5 * lz;
	xc[11] = 0 * lx;
	yc[11] = 0.3333 * ly;
	zc[11] = 0.5 * lz;
	xc[12] = 1 * lx;
	yc[12] = 0.6667 * ly;
	zc[12] = 0.5 * lz;
	xc[13] = 1 * lx;
	yc[13] = 0.3333 * ly;
	zc[13] = 0.5 * lz;
	xc[14] = 0.5 * lx;
	yc[14] = 0.1667 * ly;
	zc[14] = 0.5 * lz;
	xc[15] = 0.5 * lx;
	yc[15] = 0.8333 * ly;
	zc[15] = 0.5 * lz;

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
				for (nc = 0; nc < 16; nc++)
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

void initW_aZ(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[16], yc[16], zc[16];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aZ.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0 * lx;
	yc[0] = 0 * ly;
	zc[0] = 0 * lz;
	xc[1] = 0 * lx;
	yc[1] = 0 * ly;
	zc[1] = 1 * lz;
	xc[2] = 1 * lx;
	yc[2] = 0 * ly;
	zc[2] = 0 * lz;
	xc[3] = 1 * lx;
	yc[3] = 0 * ly;
	zc[3] = 1 * lz;
	xc[4] = 0 * lx;
	yc[4] = 1 * ly;
	zc[4] = 0 * lz;
	xc[5] = 0 * lx;
	yc[5] = 1 * ly;
	zc[5] = 1 * lz;
	xc[6] = 1 * lx;
	yc[6] = 1 * ly;
	zc[6] = 0 * lz;
	xc[7] = 1 * lx;
	yc[7] = 1 * ly;
	zc[7] = 1 * lz;
	xc[8] = 0.5 * lx;
	yc[8] = 0.5 * ly;
	zc[8] = 0 * lz;
	xc[9] = 0.5 * lx;
	yc[9] = 0.5 * ly;
	zc[9] = 1 * lz;
	xc[10] = 0 * lx;
	yc[10] = 0.6667 * ly;
	zc[10] = 0.5 * lz;

	xc[11] = 0 * lx;
	yc[11] = 0.3333 * ly;
	zc[11] = 0.5 * lz;
	xc[12] = 1 * lx;
	yc[12] = 0.6667 * ly;
	zc[12] = 0.5 * lz;
	xc[13] = 1 * lx;
	yc[13] = 0.3333 * ly;
	zc[13] = 0.5 * lz;
	xc[14] = 0.5 * lx;
	yc[14] = 0.1667 * ly;
	zc[14] = 0.5 * lz;
	xc[15] = 0.5 * lx;
	yc[15] = 0.8333 * ly;
	zc[15] = 0.5 * lz;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 1;
				for (nc = 0; nc < 10; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 10; nc < 16; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
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

void initW_aS30(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[43], yc[43], zc[43];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aS30.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (32 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (32 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0 * lx;
	yc[0] = 0 * ly;
	zc[0] = 0 * lz;
	xc[1] = 0 * lx;
	yc[1] = 0.5 * ly;
	zc[1] = 0.5 * lz;
	xc[2] = 0.5 * lx;
	yc[2] = 0 * ly;
	zc[2] = 0.5 * lz;
	xc[3] = 0.5 * lx;
	yc[3] = 0.5 * ly;
	zc[3] = 0 * lz;
	xc[4] = 0 * lx;
	yc[4] = 0 * ly;
	zc[4] = 1 * lz;
	xc[5] = 0.5 * lx;
	yc[5] = 0.5 * ly;
	zc[5] = 1 * lz;
	xc[6] = 0 * lx;
	yc[6] = 1 * ly;
	zc[6] = 0 * lz;
	xc[7] = 0.5 * lx;
	yc[7] = 1 * ly;
	zc[7] = 0.5 * lz;
	xc[8] = 0 * lx;
	yc[8] = 1 * ly;
	zc[8] = 1 * lz;
	xc[9] = 1 * lx;
	yc[9] = 0 * ly;
	zc[9] = 0 * lz;
	xc[10] = 1 * lx;
	yc[10] = 0.5 * ly;
	zc[10] = 0.5 * lz;
	xc[11] = 1 * lx;
	yc[11] = 0 * ly;
	zc[11] = 1 * lz;
	xc[12] = 1 * lx;
	yc[12] = 1 * ly;
	zc[12] = 0 * lz;
	xc[13] = 1 * lx;
	yc[13] = 1 * ly;
	zc[13] = 1 * lz;

	xc[14] = 0.5 * lx;
	yc[14] = 0.5 * ly;
	zc[14] = 0.5 * lz;
	xc[15] = 0.5 * lx;
	yc[15] = 0 * ly;
	zc[15] = 0 * lz;
	xc[16] = 0.5 * lx;
	yc[16] = 0 * ly;
	zc[16] = 1 * lz;
	xc[17] = 0.5 * lx;
	yc[17] = 1 * ly;
	zc[17] = 0 * lz;
	xc[18] = 0.5 * lx;
	yc[18] = 1 * ly;
	zc[18] = 1 * lz;
	xc[19] = 0 * lx;
	yc[19] = 0.5 * ly;
	zc[19] = 0 * lz;
	xc[20] = 0 * lx;
	yc[20] = 0.5 * ly;
	zc[20] = 1 * lz;
	xc[21] = 1 * lx;
	yc[21] = 0.5 * ly;
	zc[21] = 0 * lz;
	xc[22] = 1 * lx;
	yc[22] = 0.5 * ly;
	zc[22] = 1 * lz;
	xc[23] = 0 * lx;
	yc[23] = 0 * ly;
	zc[23] = 0.5 * lz;
	xc[24] = 0 * lx;
	yc[24] = 1 * ly;
	zc[24] = 0.5 * lz;
	xc[25] = 1 * lx;
	yc[25] = 0 * ly;
	zc[25] = 0.5 * lz;
	xc[26] = 1 * lx;
	yc[26] = 1 * ly;
	zc[26] = 0.5 * lz;
	xc[27] = 0.25 * lx;
	yc[27] = 0.25 * ly;
	zc[27] = 0.25 * lz;
	xc[28] = 0.25 * lx;
	yc[28] = 0.25 * ly;
	zc[28] = 0.75 * lz;
	xc[29] = 0.25 * lx;
	yc[29] = 0.75 * ly;
	zc[29] = 0.25 * lz;
	xc[30] = 0.25 * lx;
	yc[30] = 0.75 * ly;
	zc[30] = 0.75 * lz;
	xc[31] = 0.75 * lx;
	yc[31] = 0.25 * ly;
	zc[31] = 0.25 * lz;
	xc[32] = 0.75 * lx;
	yc[32] = 0.25 * ly;
	zc[32] = 0.75 * lz;
	xc[33] = 0.75 * lx;
	yc[33] = 0.75 * ly;
	zc[33] = 0.25 * lz;
	xc[34] = 0.75 * lx;
	yc[34] = 0.75 * ly;
	zc[34] = 0.75 * lz;
	xc[35] = 0.25 * lx;
	yc[35] = 0.25 * ly;
	zc[35] = 0.25 * lz;
	xc[36] = 0.25 * lx;
	yc[36] = 0.25 * ly;
	zc[36] = 0.75 * lz;
	xc[37] = 0.25 * lx;
	yc[37] = 0.75 * ly;
	zc[37] = 0.25 * lz;
	xc[38] = 0.25 * lx;
	yc[38] = 0.75 * ly;
	zc[38] = 0.75 * lz;
	xc[39] = 0.75 * lx;
	yc[39] = 0.25 * ly;
	zc[39] = 0.25 * lz;
	xc[40] = 0.75 * lx;
	yc[40] = 0.25 * ly;
	zc[40] = 0.75 * lz;
	xc[41] = 0.75 * lx;
	yc[41] = 0.75 * ly;
	zc[41] = 0.25 * lz;
	xc[42] = 0.75 * lx;
	yc[42] = 0.75 * ly;
	zc[42] = 0.75 * lz;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 1;
				for (nc = 0; nc < 14; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 14; nc < 43; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
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

void initW_aS24(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[34], yc[34], zc[34];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aS30.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (32 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (32 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0 * lx;
	yc[0] = 0 * ly;
	zc[0] = 0 * lz;
	xc[1] = 0 * lx;
	yc[1] = 0 * ly;
	zc[1] = 1 * lz;
	xc[2] = 0 * lx;
	yc[2] = 1 * ly;
	zc[2] = 0 * lz;
	xc[3] = 0 * lx;
	yc[3] = 0.5 * ly;
	zc[3] = 0.5 * lz;
	xc[4] = 0 * lx;
	yc[4] = 1 * ly;
	zc[4] = 1 * lz;
	xc[5] = 0.25 * lx;
	yc[5] = 0.75 * ly;
	zc[5] = 0.75 * lz;
	xc[6] = 1 * lx;
	yc[6] = 0 * ly;
	zc[6] = 0 * lz;
	xc[7] = 1 * lx;
	yc[7] = 0 * ly;
	zc[7] = 1 * lz;
	xc[8] = 0.5 * lx;
	yc[8] = 0 * ly;
	zc[8] = 0.5 * lz;
	xc[9] = 0.75 * lx;
	yc[9] = 0.25 * ly;
	zc[9] = 0.75 * lz;
	xc[10] = 1 * lx;
	yc[10] = 1 * ly;
	zc[10] = 0 * lz;
	xc[11] = 0.5 * lx;
	yc[11] = 0.5 * ly;
	zc[11] = 0 * lz;
	xc[12] = 0.75 * lx;
	yc[12] = 0.75 * ly;
	zc[12] = 0.25 * lz;
	xc[13] = 1 * lx;
	yc[13] = 0.5 * ly;
	zc[13] = 0.5 * lz;
	xc[14] = 1 * lx;
	yc[14] = 1 * ly;
	zc[14] = 1 * lz;
	xc[15] = 0.5 * lx;
	yc[15] = 1 * ly;
	zc[15] = 0.5 * lz;
	xc[16] = 0.5 * lx;
	yc[16] = 0.5 * ly;
	zc[16] = 1 * lz;
	xc[17] = 0.25 * lx;
	yc[17] = 0.25 * ly;
	zc[17] = 0.25 * lz;

	xc[18] = 0.125 * lx;
	yc[18] = 0.375 * ly;
	zc[18] = 0.875 * lz;
	xc[19] = 0.375 * lx;
	yc[19] = 0.125 * ly;
	zc[19] = 0.875 * lz;
	xc[20] = 0.125 * lx;
	yc[20] = 0.125 * ly;
	zc[20] = 0.625 * lz;
	xc[21] = 0.375 * lx;
	yc[21] = 0.875 * ly;
	zc[21] = 0.125 * lz;
	xc[22] = 0.125 * lx;
	yc[22] = 0.875 * ly;
	zc[22] = 0.375 * lz;
	xc[23] = 0.125 * lx;
	yc[23] = 0.625 * ly;
	zc[23] = 0.125 * lz;
	xc[24] = 0.375 * lx;
	yc[24] = 0.625 * ly;
	zc[24] = 0.375 * lz;
	xc[25] = 0.375 * lx;
	yc[25] = 0.375 * ly;
	zc[25] = 0.625 * lz;
	xc[26] = 0.875 * lx;
	yc[26] = 0.375 * ly;
	zc[26] = 0.125 * lz;
	xc[27] = 0.875 * lx;
	yc[27] = 0.125 * ly;
	zc[27] = 0.375 * lz;
	xc[28] = 0.625 * lx;
	yc[28] = 0.125 * ly;
	zc[28] = 0.125 * lz;
	xc[29] = 0.625 * lx;
	yc[29] = 0.375 * ly;
	zc[29] = 0.375 * lz;
	xc[30] = 0.625 * lx;
	yc[30] = 0.625 * ly;
	zc[30] = 0.625 * lz;
	xc[31] = 0.625 * lx;
	yc[31] = 0.875 * ly;
	zc[31] = 0.875 * lz;
	xc[32] = 0.875 * lx;
	yc[32] = 0.875 * ly;
	zc[32] = 0.625 * lz;
	xc[33] = 0.875 * lx;
	yc[33] = 0.625 * ly;
	zc[33] = 0.875 * lz;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 1;
				for (nc = 0; nc < 18; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 18; nc < 34; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
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

void initW_csC14(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[38], yc[38], zc[38];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csC14.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.5 * lx;
	yc[0] = 0.833333335 * ly;
	zc[0] = 0.06680525 * lz;
	xc[1] = 0 * lx;
	yc[1] = 0.333333335 * ly;
	zc[1] = 0.06680525 * lz;
	xc[2] = 1 * lx;
	yc[2] = 0.66666667 * ly;
	zc[2] = 0.56680525 * lz;
	xc[3] = 0.5 * lx;
	yc[3] = 0.16666667 * ly;
	zc[3] = 0.56680525 * lz;
	xc[4] = 1 * lx;
	yc[4] = 0.66666667 * ly;
	zc[4] = 0.93319475 * lz;
	xc[5] = 0.5 * lx;
	yc[5] = 0.16666667 * ly;
	zc[5] = 0.93319475 * lz;
	xc[6] = 0.5 * lx;
	yc[6] = 0.833333335 * ly;
	zc[6] = 0.43319475 * lz;
	xc[7] = 0 * lx;
	yc[7] = 0.333333335 * ly;
	zc[7] = 0.43319475 * lz;
	xc[8] = 1 * lx;
	yc[8] = 1 * ly;
	zc[8] = 0 * lz;
	xc[9] = 1 * lx;
	yc[9] = 0.83011583 * ly;
	zc[9] = 0.25 * lz;
	xc[10] = 1 * lx;
	yc[10] = 1 * ly;
	zc[10] = 0.5 * lz;
	xc[11] = 0.754826255 * lx;
	yc[11] = 0.584942085 * ly;
	zc[11] = 0.25 * lz;
	xc[12] = 0.245173745 * lx;
	yc[12] = 0.584942085 * ly;
	zc[12] = 0.25 * lz;
	xc[13] = 1 * lx;
	yc[13] = 1 * ly;
	zc[13] = 1 * lz;
	xc[14] = 0.254826255 * lx;
	yc[14] = 0.915057915 * ly;
	zc[14] = 0.75 * lz;
	xc[15] = 0.745173745 * lx;
	yc[15] = 0.915057915 * ly;
	zc[15] = 0.75 * lz;
	xc[16] = 0.5 * lx;
	yc[16] = 0.66988417 * ly;
	zc[16] = 0.75 * lz;
	xc[17] = 1 * lx;
	yc[17] = 1 * ly;
	zc[17] = 0 * lz;
	xc[18] = 1 * lx;
	yc[18] = 1 * ly;
	zc[18] = 0.5 * lz;
	xc[19] = 1 * lx;
	yc[19] = 1 * ly;
	zc[19] = 1 * lz;
	xc[20] = 0.5 * lx;
	yc[20] = 0.5 * ly;
	zc[20] = 0 * lz;
	xc[21] = 0.5 * lx;
	yc[21] = 0.33011583 * ly;
	zc[21] = 0.25 * lz;
	xc[22] = 0.5 * lx;
	yc[22] = 0.5 * ly;
	zc[22] = 0.5 * lz;
	xc[23] = 0.254826255 * lx;
	yc[23] = 0.084942085 * ly;
	zc[23] = 0.25 * lz;
	xc[24] = 0.745173745 * lx;
	yc[24] = 0.084942085 * ly;
	zc[24] = 0.25 * lz;
	xc[25] = 0.5 * lx;
	yc[25] = 0.5 * ly;
	zc[25] = 1 * lz;
	xc[26] = 0.754826255 * lx;
	yc[26] = 0.415057915 * ly;
	zc[26] = 0.75 * lz;
	xc[27] = 0.245173745 * lx;
	yc[27] = 0.415057915 * ly;
	zc[27] = 0.75 * lz;
	xc[28] = 1 * lx;
	yc[28] = 0.16988417 * ly;
	zc[28] = 0.75 * lz;
	xc[29] = 0.5 * lx;
	yc[29] = 0.5 * ly;
	zc[29] = 0 * lz;
	xc[30] = 0.5 * lx;
	yc[30] = 0.5 * ly;
	zc[30] = 0.5 * lz;
	xc[31] = 0.5 * lx;
	yc[31] = 0.5 * ly;
	zc[31] = 1 * lz;
	xc[32] = 0 * lx;
	yc[32] = 0 * ly;
	zc[32] = 0 * lz;
	xc[33] = 0 * lx;
	yc[33] = 0 * ly;
	zc[33] = 0.5 * lz;
	xc[34] = 0 * lx;
	yc[34] = 0 * ly;
	zc[34] = 1 * lz;
	xc[35] = 1 * lx;
	yc[35] = 0 * ly;
	zc[35] = 0 * lz;
	xc[36] = 1 * lx;
	yc[36] = 0 * ly;
	zc[36] = 0.5 * lz;
	xc[37] = 1 * lx;
	yc[37] = 0 * ly;
	zc[37] = 1 * lz;

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
				for (nc = 0; nc < 38; nc++)
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

void initW_csC14_z(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[25], yc[25], zc[25];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csC14_z.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	zc[0] = 0.06680525 * lz;
	yc[0] = 0.66666667 * ly;
	xc[0] = 0 * lx;
	zc[1] = 0 * lz;
	yc[1] = 0 * ly;
	xc[1] = 0 * lx;
	zc[2] = 0.25 * lz;
	yc[2] = 0.16988417 * ly;
	xc[2] = 0.254826255 * lx;
	zc[3] = 0.25 * lz;
	yc[3] = 0.16988417 * ly;
	xc[3] = 0.745173745 * lx;
	zc[4] = 0.25 * lz;
	yc[4] = 0.66023166 * ly;
	xc[4] = 0.5 * lx;
	zc[5] = 0.75 * lz;
	yc[5] = 0.83011583 * ly;
	xc[5] = 0.754826255 * lx;
	zc[6] = 0.75 * lz;
	yc[6] = 0.83011583 * ly;
	xc[6] = 0.245173745 * lx;
	zc[7] = 0.75 * lz;
	yc[7] = 0.33976834 * ly;
	xc[7] = 0 * lx;
	zc[8] = 0.5 * lz;
	yc[8] = 0 * ly;
	xc[8] = 0 * lx;
	zc[9] = 0.5 * lz;
	yc[9] = 0 * ly;
	xc[9] = 0 * lx;
	zc[10] = 0.25 * lz;
	yc[10] = 0.16988417 * ly;
	xc[10] = 0.254826255 * lx;
	zc[11] = 0.25 * lz;
	yc[11] = 0.16988417 * ly;
	xc[11] = 0.254826255 * lx;
	zc[12] = 0.75 * lz;
	yc[12] = 0.33976834 * ly;
	xc[12] = 0 * lx;
	zc[13] = 0.75 * lz;
	yc[13] = 0.33976834 * ly;
	xc[13] = 0 * lx;
	zc[14] = 0.75 * lz;
	yc[14] = 0.83011583 * ly;
	xc[14] = 0.754826255 * lx;
	zc[15] = 0.75 * lz;
	yc[15] = 0.83011583 * ly;
	xc[15] = 0.754826255 * lx;
	zc[16] = 0.25 * lz;
	yc[16] = 0.16988417 * ly;
	xc[16] = 0.745173745 * lx;
	zc[17] = 0.25 * lz;
	yc[17] = 0.16988417 * ly;
	xc[17] = 0.745173745 * lx;
	zc[18] = 0.25 * lz;
	yc[18] = 0.66023166 * ly;
	xc[18] = 0.5 * lx;
	zc[19] = 0.25 * lz;
	yc[19] = 0.66023166 * ly;
	xc[19] = 0.5 * lx;
	zc[20] = 0.75 * lz;
	yc[20] = 0.83011583 * ly;
	xc[20] = 0.245173745 * lx;
	zc[21] = 0.75 * lz;
	yc[21] = 0.83011583 * ly;
	xc[21] = 0.245173745 * lx;
	zc[22] = 0.43319475 * lz;
	yc[22] = 0.66666667 * ly;
	xc[22] = 0 * lx;
	zc[23] = 0.56680525 * lz;
	yc[23] = 0.33333333 * ly;
	xc[23] = 0.5 * lx;
	zc[24] = 0.93319475 * lz;
	yc[24] = 0.33333333 * ly;
	xc[24] = 0.5 * lx;

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
				for (nc = 0; nc < 25; nc++)
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

void initW_csC15(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[50], yc[50], zc[50];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csC15.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.625 * lx;
	yc[0] = 0.625 * ly;
	zc[0] = 0.625 * lz;
	xc[1] = 0 * lx;
	yc[1] = 0.5 * ly;
	zc[1] = 0.5 * lz;
	xc[2] = 0.625 * lx;
	yc[2] = 0.875 * ly;
	zc[2] = 0.875 * lz;
	xc[3] = 0 * lx;
	yc[3] = 0 * ly;
	zc[3] = 0 * lz;
	xc[4] = 0.5 * lx;
	yc[4] = 0 * ly;
	zc[4] = 0.5 * lz;
	xc[5] = 0.5 * lx;
	yc[5] = 0.5 * ly;
	zc[5] = 0 * lz;
	xc[6] = 0.875 * lx;
	yc[6] = 0.875 * ly;
	zc[6] = 0.625 * lz;
	xc[7] = 0.875 * lx;
	yc[7] = 0.625 * ly;
	zc[7] = 0.875 * lz;
	xc[8] = 0.375 * lx;
	yc[8] = 0.875 * ly;
	zc[8] = 0.125 * lz;
	xc[9] = 0.375 * lx;
	yc[9] = 0.875 * ly;
	zc[9] = 0.125 * lz;
	xc[10] = 0.375 * lx;
	yc[10] = 0.875 * ly;
	zc[10] = 0.125 * lz;
	xc[11] = 0.375 * lx;
	yc[11] = 0.625 * ly;
	zc[11] = 0.375 * lz;
	xc[12] = 0.375 * lx;
	yc[12] = 0.625 * ly;
	zc[12] = 0.375 * lz;
	xc[13] = 0.375 * lx;
	yc[13] = 0.625 * ly;
	zc[13] = 0.375 * lz;
	xc[14] = 0.125 * lx;
	yc[14] = 0.375 * ly;
	zc[14] = 0.875 * lz;
	xc[15] = 0.125 * lx;
	yc[15] = 0.375 * ly;
	zc[15] = 0.875 * lz;
	xc[16] = 0.125 * lx;
	yc[16] = 0.375 * ly;
	zc[16] = 0.875 * lz;
	xc[17] = 0.875 * lx;
	yc[17] = 0.375 * ly;
	zc[17] = 0.125 * lz;
	xc[18] = 0.875 * lx;
	yc[18] = 0.375 * ly;
	zc[18] = 0.125 * lz;
	xc[19] = 0.875 * lx;
	yc[19] = 0.375 * ly;
	zc[19] = 0.125 * lz;
	xc[20] = 0.875 * lx;
	yc[20] = 0.125 * ly;
	zc[20] = 0.375 * lz;
	xc[21] = 0.875 * lx;
	yc[21] = 0.125 * ly;
	zc[21] = 0.375 * lz;
	xc[22] = 0.875 * lx;
	yc[22] = 0.125 * ly;
	zc[22] = 0.375 * lz;
	xc[23] = 0.625 * lx;
	yc[23] = 0.125 * ly;
	zc[23] = 0.125 * lz;
	xc[24] = 0.625 * lx;
	yc[24] = 0.125 * ly;
	zc[24] = 0.125 * lz;
	xc[25] = 0.625 * lx;
	yc[25] = 0.125 * ly;
	zc[25] = 0.125 * lz;
	xc[26] = 0.125 * lx;
	yc[26] = 0.875 * ly;
	zc[26] = 0.375 * lz;
	xc[27] = 0.125 * lx;
	yc[27] = 0.875 * ly;
	zc[27] = 0.375 * lz;
	xc[28] = 0.125 * lx;
	yc[28] = 0.875 * ly;
	zc[28] = 0.375 * lz;
	xc[29] = 0.625 * lx;
	yc[29] = 0.375 * ly;
	zc[29] = 0.375 * lz;
	xc[30] = 0.625 * lx;
	yc[30] = 0.375 * ly;
	zc[30] = 0.375 * lz;
	xc[31] = 0.625 * lx;
	yc[31] = 0.375 * ly;
	zc[31] = 0.375 * lz;
	xc[32] = 0.375 * lx;
	yc[32] = 0.125 * ly;
	zc[32] = 0.875 * lz;
	xc[33] = 0.375 * lx;
	yc[33] = 0.125 * ly;
	zc[33] = 0.875 * lz;
	xc[34] = 0.375 * lx;
	yc[34] = 0.125 * ly;
	zc[34] = 0.875 * lz;
	xc[35] = 0.375 * lx;
	yc[35] = 0.375 * ly;
	zc[35] = 0.625 * lz;
	xc[36] = 0.375 * lx;
	yc[36] = 0.375 * ly;
	zc[36] = 0.625 * lz;
	xc[37] = 0.375 * lx;
	yc[37] = 0.375 * ly;
	zc[37] = 0.625 * lz;
	xc[38] = 0.125 * lx;
	yc[38] = 0.625 * ly;
	zc[38] = 0.125 * lz;
	xc[39] = 0.125 * lx;
	yc[39] = 0.625 * ly;
	zc[39] = 0.125 * lz;
	xc[40] = 0.125 * lx;
	yc[40] = 0.625 * ly;
	zc[40] = 0.125 * lz;
	xc[41] = 0.125 * lx;
	yc[41] = 0.125 * ly;
	zc[41] = 0.625 * lz;
	xc[42] = 0.125 * lx;
	yc[42] = 0.125 * ly;
	zc[42] = 0.625 * lz;
	xc[43] = 0.125 * lx;
	yc[43] = 0.125 * ly;
	zc[43] = 0.625 * lz;
	xc[44] = 0.75 * lx;
	yc[44] = 0.25 * ly;
	zc[44] = 0.75 * lz;
	xc[45] = 0.75 * lx;
	yc[45] = 0.25 * ly;
	zc[45] = 0.75 * lz;
	xc[46] = 0.25 * lx;
	yc[46] = 0.75 * ly;
	zc[46] = 0.75 * lz;
	xc[47] = 0.25 * lx;
	yc[47] = 0.75 * ly;
	zc[47] = 0.75 * lz;
	xc[48] = 0.75 * lx;
	yc[48] = 0.75 * ly;
	zc[48] = 0.25 * lz;
	xc[49] = 0.75 * lx;
	yc[49] = 0.75 * ly;
	zc[49] = 0.25 * lz;

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
				for (nc = 0; nc < 50; nc++)
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

void initW_csC36(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[79], yc[79], zc[79];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csC36.dat", "w");

	ra = 0.75 * pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = 0.75 * pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 1 * lx;
	yc[0] = 1 * ly;
	zc[0] = 0.094 * lz;
	xc[1] = 1 * lx;
	yc[1] = 1 * ly;
	zc[1] = 0.094 * lz;
	xc[2] = 0.5 * lx;
	yc[2] = 1 * ly;
	zc[2] = 0 * lz;
	xc[3] = 1 * lx;
	yc[3] = 1 * ly;
	zc[3] = 0.906 * lz;
	xc[4] = 0.5 * lx;
	yc[4] = 1 * ly;
	zc[4] = 1 * lz;
	xc[5] = 1 * lx;
	yc[5] = 1 * ly;
	zc[5] = 0.906 * lz;
	xc[6] = 0.5 * lx;
	yc[6] = 0.5 * ly;
	zc[6] = 0.094 * lz;
	xc[7] = 0.75 * lx;
	yc[7] = 0.75 * ly;
	zc[7] = 0 * lz;
	xc[8] = 1 * lx;
	yc[8] = 0.6667 * ly;
	zc[8] = 0.1558 * lz;
	xc[9] = 0.75 * lx;
	yc[9] = 0.75 * ly;
	zc[9] = 1 * lz;
	xc[10] = 0.50005 * lx;
	yc[10] = 0.83335 * ly;
	zc[10] = 0.12514 * lz;
	xc[11] = 0.246435 * lx;
	yc[11] = 0.917855 * ly;
	zc[11] = 0.25 * lz;
	xc[12] = 0.50005 * lx;
	yc[12] = 0.83335 * ly;
	zc[12] = 0.37486 * lz;
	xc[13] = 0.5 * lx;
	yc[13] = 0.5 * ly;
	zc[13] = 0.094 * lz;
	xc[14] = 1 * lx;
	yc[14] = 1 * ly;
	zc[14] = 0.406 * lz;
	xc[15] = 0.5 * lx;
	yc[15] = 0.66429 * ly;
	zc[15] = 0.25 * lz;
	xc[16] = 0.753565 * lx;
	yc[16] = 0.917855 * ly;
	zc[16] = 0.25 * lz;
	xc[17] = 0.75 * lx;
	yc[17] = 0.75 * ly;
	zc[17] = 0 * lz;
	xc[18] = 1 * lx;
	yc[18] = 0.5 * ly;
	zc[18] = 0 * lz;
	xc[19] = 0.25 * lx;
	yc[19] = 0.75 * ly;
	zc[19] = 0 * lz;
	xc[20] = 0 * lx;
	yc[20] = 0.66665 * ly;
	zc[20] = 0.3442 * lz;
	xc[21] = 1 * lx;
	yc[21] = 0.6667 * ly;
	zc[21] = 0.87486 * lz;
	xc[22] = 1 * lx;
	yc[22] = 1 * ly;
	zc[22] = 0.594 * lz;
	xc[23] = 0.5 * lx;
	yc[23] = 0.5 * ly;
	zc[23] = 0.906 * lz;
	xc[24] = 0.746435 * lx;
	yc[24] = 0.582145 * ly;
	zc[24] = 0.75 * lz;
	xc[25] = 1 * lx;
	yc[25] = 0.83571 * ly;
	zc[25] = 0.75 * lz;
	xc[26] = 0.75 * lx;
	yc[26] = 0.75 * ly;
	zc[26] = 1 * lz;
	xc[27] = 1 * lx;
	yc[27] = 0.5 * ly;
	zc[27] = 1 * lz;
	xc[28] = 0.25 * lx;
	yc[28] = 0.75 * ly;
	zc[28] = 1 * lz;
	xc[29] = 1 * lx;
	yc[29] = 1 * ly;
	zc[29] = 0.406 * lz;
	xc[30] = 0.5 * lx;
	yc[30] = 0.8333 * ly;
	zc[30] = 0.8442 * lz;
	xc[31] = 1 * lx;
	yc[31] = 1 * ly;
	zc[31] = 0.594 * lz;
	xc[32] = 0.5 * lx;
	yc[32] = 0.5 * ly;
	zc[32] = 0.906 * lz;
	xc[33] = 0.253565 * lx;
	yc[33] = 0.582145 * ly;
	zc[33] = 0.75 * lz;
	xc[34] = 0.49995 * lx;
	yc[34] = 0.83335 * ly;
	zc[34] = 0.6558 * lz;
	xc[35] = 0 * lx;
	yc[35] = 0 * ly;
	zc[35] = 0.094 * lz;
	xc[36] = 0.25 * lx;
	yc[36] = 0.25 * ly;
	zc[36] = 0 * lz;
	xc[37] = 0.5 * lx;
	yc[37] = 0.1667 * ly;
	zc[37] = 0.1558 * lz;
	xc[38] = 0.25 * lx;
	yc[38] = 0.25 * ly;
	zc[38] = 1 * lz;
	xc[39] = 0 * lx;
	yc[39] = 0.33335 * ly;
	zc[39] = 0.12514 * lz;
	xc[40] = 0.746435 * lx;
	yc[40] = 0.417855 * ly;
	zc[40] = 0.25 * lz;
	xc[41] = 0 * lx;
	yc[41] = 0.33335 * ly;
	zc[41] = 0.37486 * lz;
	xc[42] = 1 * lx;
	yc[42] = 0 * ly;
	zc[42] = 0.094 * lz;
	xc[43] = 0.5 * lx;
	yc[43] = 0.5 * ly;
	zc[43] = 0.406 * lz;
	xc[44] = 1 * lx;
	yc[44] = 0.16429 * ly;
	zc[44] = 0.25 * lz;
	xc[45] = 0.253565 * lx;
	yc[45] = 0.417855 * ly;
	zc[45] = 0.25 * lz;
	xc[46] = 0.25 * lx;
	yc[46] = 0.25 * ly;
	zc[46] = 0 * lz;
	xc[47] = 0.5 * lx;
	yc[47] = 0 * ly;
	zc[47] = 0 * lz;
	xc[48] = 0.75 * lx;
	yc[48] = 0.25 * ly;
	zc[48] = 0 * lz;
	xc[49] = 0.50005 * lx;
	yc[49] = 0.16665 * ly;
	zc[49] = 0.3442 * lz;
	xc[50] = 0.5 * lx;
	yc[50] = 0.1667 * ly;
	zc[50] = 0.87486 * lz;
	xc[51] = 0.5 * lx;
	yc[51] = 0.5 * ly;
	zc[51] = 0.594 * lz;
	xc[52] = 0 * lx;
	yc[52] = 0 * ly;
	zc[52] = 0.906 * lz;
	xc[53] = 0.246435 * lx;
	yc[53] = 0.082145 * ly;
	zc[53] = 0.75 * lz;
	xc[54] = 0.5 * lx;
	yc[54] = 0.33571 * ly;
	zc[54] = 0.75 * lz;
	xc[55] = 0.25 * lx;
	yc[55] = 0.25 * ly;
	zc[55] = 1 * lz;
	xc[56] = 0.5 * lx;
	yc[56] = 0 * ly;
	zc[56] = 1 * lz;
	xc[57] = 0.75 * lx;
	yc[57] = 0.25 * ly;
	zc[57] = 1 * lz;
	xc[58] = 0.5 * lx;
	yc[58] = 0.5 * ly;
	zc[58] = 0.406 * lz;
	xc[59] = 1 * lx;
	yc[59] = 0.3333 * ly;
	zc[59] = 0.8442 * lz;
	xc[60] = 1 * lx;
	yc[60] = 0.6667 * ly;
	zc[60] = 0.62514 * lz;
	xc[61] = 0.5 * lx;
	yc[61] = 0.5 * ly;
	zc[61] = 0.594 * lz;
	xc[62] = 1 * lx;
	yc[62] = 0 * ly;
	zc[62] = 0.906 * lz;
	xc[63] = 0.753565 * lx;
	yc[63] = 0.082145 * ly;
	zc[63] = 0.75 * lz;
	xc[64] = 0.99995 * lx;
	yc[64] = 0.33335 * ly;
	zc[64] = 0.6558 * lz;
	xc[65] = 0 * lx;
	yc[65] = 0 * ly;
	zc[65] = 0.406 * lz;
	xc[66] = 0 * lx;
	yc[66] = 0 * ly;
	zc[66] = 0.594 * lz;
	xc[67] = 1 * lx;
	yc[67] = 0 * ly;
	zc[67] = 0.406 * lz;
	xc[68] = 0.5 * lx;
	yc[68] = 0.1667 * ly;
	zc[68] = 0.62514 * lz;
	xc[69] = 1 * lx;
	yc[69] = 0 * ly;
	zc[69] = 0.594 * lz;
	xc[70] = 0.25 * lx;
	yc[70] = 0.75 * ly;
	zc[70] = 0.5 * lz;
	xc[71] = 0.75 * lx;
	yc[71] = 0.75 * ly;
	zc[71] = 0.5 * lz;
	xc[72] = 0.5 * lx;
	yc[72] = 1 * ly;
	zc[72] = 0.5 * lz;
	xc[73] = 0.75 * lx;
	yc[73] = 0.75 * ly;
	zc[73] = 0.5 * lz;
	xc[74] = 0.75 * lx;
	yc[74] = 0.25 * ly;
	zc[74] = 0.5 * lz;
	xc[75] = 0.25 * lx;
	yc[75] = 0.25 * ly;
	zc[75] = 0.5 * lz;
	xc[76] = 1 * lx;
	yc[76] = 0.5 * ly;
	zc[76] = 0.5 * lz;
	xc[77] = 0.25 * lx;
	yc[77] = 0.25 * ly;
	zc[77] = 0.5 * lz;
	xc[78] = 0.5 * lx;
	yc[78] = 0 * ly;
	zc[78] = 0.5 * lz;

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
				for (nc = 0; nc < 79; nc++)
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

void initW_csSigma_z(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[47], yc[47], zc[47];
	double xi, yj, zk, rij, rb, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csSigma_z.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (64 * 4.0 * Pi / 3.0)), 1.0 / 3);

	zc[0] = 0.0 * lz;
	yc[0] = 0.0 * ly;
	xc[0] = 0.0 * lx;
	zc[30] = 1.0 * lz;
	yc[30] = 0.0 * ly;
	xc[30] = 0.0 * lx;
	zc[31] = 0.0 * lz;
	yc[31] = 1.0 * ly;
	xc[31] = 0.0 * lx;
	zc[32] = 1.0 * lz;
	yc[32] = 1.0 * ly;
	xc[32] = 0.0 * lx;
	zc[33] = 0.0 * lz;
	yc[33] = 0.0 * ly;
	xc[33] = 1.0 * lx;
	zc[34] = 1.0 * lz;
	yc[34] = 0.0 * ly;
	xc[34] = 1.0 * lx;
	zc[35] = 0.0 * lz;
	yc[35] = 1.0 * ly;
	xc[35] = 1.0 * lx;
	zc[36] = 1.0 * lz;
	yc[36] = 1.0 * ly;
	xc[36] = 1.0 * lx;
	zc[1] = 0.5 * lz;
	yc[1] = 0.5 * ly;
	xc[1] = 0.5 * lx;
	zc[2] = 0.31726 * lz;
	yc[2] = 0.68274 * ly;
	xc[2] = 0.24834 * lx;
	zc[3] = 0.68274 * lz;
	yc[3] = 0.31726 * ly;
	xc[3] = 0.24834 * lx;
	zc[4] = 0.18274 * lz;
	yc[4] = 0.18274 * ly;
	xc[4] = 0.25166 * lx;
	zc[5] = 0.81726 * lz;
	yc[5] = 0.81726 * ly;
	xc[5] = 0.25166 * lx;
	zc[6] = 0.18274 * lz;
	yc[6] = 0.18274 * ly;
	xc[6] = 0.74834 * lx;
	zc[7] = 0.81726 * lz;
	yc[7] = 0.81726 * ly;
	xc[7] = 0.74834 * lx;
	zc[8] = 0.31726 * lz;
	yc[8] = 0.68274 * ly;
	xc[8] = 0.75166 * lx;
	zc[9] = 0.68274 * lz;
	yc[9] = 0.31726 * ly;
	xc[9] = 0.75166 * lx;
	zc[10] = 0.39875 * lz;
	yc[10] = 0.39875 * ly;
	xc[10] = 0.0 * lx;
	zc[37] = 0.39875 * lz;
	yc[37] = 0.39875 * ly;
	xc[37] = 1.0 * lx;
	zc[11] = 0.60125 * lz;
	yc[11] = 0.60125 * ly;
	xc[11] = 0.0 * lx;
	zc[38] = 0.60125 * lz;
	yc[38] = 0.60125 * ly;
	xc[38] = 1.0 * lx;
	zc[12] = 0.89875 * lz;
	yc[12] = 0.10125 * ly;
	xc[12] = 0.5 * lx;
	zc[13] = 0.10125 * lz;
	yc[13] = 0.89875 * ly;
	xc[13] = 0.5 * lx;
	zc[14] = 0.73921 * lz;
	yc[14] = 0.06608 * ly;
	xc[14] = 0.0 * lx;
	zc[15] = 0.93392 * lz;
	yc[15] = 0.26079 * ly;
	xc[15] = 0.0 * lx;
	zc[16] = 0.06608 * lz;
	yc[16] = 0.73921 * ly;
	xc[16] = 0.0 * lx;
	zc[17] = 0.26079 * lz;
	yc[17] = 0.93392 * ly;
	xc[17] = 0.0 * lx;
	zc[39] = 0.73921 * lz;
	yc[39] = 0.06608 * ly;
	xc[39] = 1.0 * lx;
	zc[40] = 0.93392 * lz;
	yc[40] = 0.26079 * ly;
	xc[40] = 1.0 * lx;
	zc[41] = 0.06608 * lz;
	yc[41] = 0.73921 * ly;
	xc[41] = 1.0 * lx;
	zc[42] = 0.26079 * lz;
	yc[42] = 0.93392 * ly;
	xc[42] = 1.0 * lx;
	zc[18] = 0.43392 * lz;
	yc[18] = 0.23921 * ly;
	xc[18] = 0.5 * lx;
	zc[19] = 0.23921 * lz;
	yc[19] = 0.43392 * ly;
	xc[19] = 0.5 * lx;
	zc[20] = 0.76079 * lz;
	yc[20] = 0.56608 * ly;
	xc[20] = 0.5 * lx;
	zc[21] = 0.56608 * lz;
	yc[21] = 0.76079 * ly;
	xc[21] = 0.5 * lx;
	zc[22] = 0.13131 * lz;
	yc[22] = 0.46351 * ly;
	xc[22] = 0.0 * lx;
	zc[23] = 0.46351 * lz;
	yc[23] = 0.13131 * ly;
	xc[23] = 0.0 * lx;
	zc[24] = 0.53649 * lz;
	yc[24] = 0.86869 * ly;
	xc[24] = 0.0 * lx;
	zc[25] = 0.86869 * lz;
	yc[25] = 0.53649 * ly;
	xc[25] = 0.0 * lx;
	zc[43] = 0.13131 * lz;
	yc[43] = 0.46351 * ly;
	xc[43] = 1.0 * lx;
	zc[44] = 0.46351 * lz;
	yc[44] = 0.13131 * ly;
	xc[44] = 1.0 * lx;
	zc[45] = 0.53649 * lz;
	yc[45] = 0.86869 * ly;
	xc[45] = 1.0 * lx;
	zc[46] = 0.86869 * lz;
	yc[46] = 0.53649 * ly;
	xc[46] = 1.0 * lx;
	zc[26] = 0.03649 * lz;
	yc[26] = 0.63131 * ly;
	xc[26] = 0.5 * lx;
	zc[27] = 0.36869 * lz;
	yc[27] = 0.96351 * ly;
	xc[27] = 0.5 * lx;
	zc[28] = 0.63131 * lz;
	yc[28] = 0.03649 * ly;
	xc[28] = 0.5 * lx;
	zc[29] = 0.96351 * lz;
	yc[29] = 0.36869 * ly;
	xc[29] = 0.5 * lx;

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
				for (nc = 0; nc < 47; nc++)
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

void initW_aBCC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[9], yc[9], zc[9];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aBCC.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (4 * 4.0 * Pi / 3.0)), 1.0 / 3);

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
				tag = 1;
				for (nc = 0; nc < 8; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
				}
				for (nc = 8; nc < 9; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
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

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (8 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rb = pow(((fAinit + fBinit) / (fA + fB + fC) * lx * ly * lz / (8 * 4.0 * Pi / 3.0)), 1.0 / 3);

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
	zc[8] = 0.0;
	xc[9] = lx / 2;
	yc[9] = ly / 2;
	zc[9] = lz;
	xc[10] = 0.0;
	yc[10] = ly / 2;
	zc[10] = lz / 2;
	xc[11] = lx;
	yc[11] = ly / 2;
	zc[11] = lz / 2;
	xc[12] = lx / 2;
	yc[12] = 0.0;
	zc[12] = lz / 2;
	xc[13] = lx / 2;
	yc[13] = ly;
	zc[13] = lz / 2;

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
				for (nc = 0; nc < 14; nc++)
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

void initW_aFCC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[15], yc[15], zc[15];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aFCC.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (8 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (8 * 4.0 * Pi / 3.0)), 1.0 / 3);

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
	zc[8] = 0.0;
	xc[9] = lx / 2;
	yc[9] = ly / 2;
	zc[9] = lz;
	xc[10] = 0.0;
	yc[10] = ly / 2;
	zc[10] = lz / 2;
	xc[11] = lx;
	yc[11] = ly / 2;
	zc[11] = lz / 2;
	xc[12] = lx / 2;
	yc[12] = 0.0;
	zc[12] = lz / 2;
	xc[13] = lx / 2;
	yc[13] = ly;
	zc[13] = lz / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 1;
				for (nc = 0; nc < 12; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
				}
				for (nc = 12; nc < 14; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
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

void initW_aG(double *wA, double *wB, double *wC)
{
	int i, j, k, tag;
	long ijk;
	double xi, yj, zk, rA, rB, sincos;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aG.dat", "w");

	rA = (fA / (fA + fB + fC) * 3.0 - 1.5) / 2;
	rB = ((fA + fB) / (fA + fB + fC) * 3.0 - 1.5) / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = (i - Nx / 2.0) / Nx * 2.0 * Pi;
		for (j = 0; j < Ny; j++)
		{
			yj = (j - Ny / 2.0) / Ny * 2.0 * Pi;
			for (k = 0; k < Nz; k++)
			{
				zk = (k - Nz / 2.0) / Nz * 2.0 * Pi;
				tag = 0;

				sincos = sin(xi) * cos(yj) + sin(yj) * cos(zk) + sin(zk) * cos(xi);

				if (sincos < rB)
				{
					tag = 1;
					if (sincos < rA)
					{
						tag = 2;
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

void initW_csG(double *wA, double *wB, double *wC)
{
	int i, j, k, tag;
	long ijk;
	double xi, yj, zk, rC, rB, sincos;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csG.dat", "w");

	rC = (fC / (fA + fB + fC) * 3.0) / 2;
	rB = ((fC + fB) / (fA + fB + fC) * 3.0) / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = (i - Nx / 2.0) / Nx * 2.0 * Pi;
		for (j = 0; j < Ny; j++)
		{
			yj = (j - Ny / 2.0) / Ny * 2.0 * Pi;
			for (k = 0; k < Nz; k++)
			{
				zk = (k - Nz / 2.0) / Nz * 2.0 * Pi;
				tag = 0;

				sincos = sin(xi) * cos(yj) + sin(yj) * cos(zk) + sin(zk) * cos(xi);

				if (abs(sincos) > rC)
				{
					tag = 1;
					if (abs(sincos) > rB)
					{
						tag = 2;
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
	fp = fopen("init_csC.dat", "w");

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

void initW_aC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[5], yc[5];
	double xi, yj, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aC.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fCinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

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
				tag = 1;
				for (nc = 0; nc < 4; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
				}
				for (nc = 4; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
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
void initW_aC42(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[8], yc[8];
	double xi, yj, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aC42.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fCinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

	xc[0] = 0.0;
	yc[0] = 0.0;
	xc[1] = 0.0;
	yc[1] = ly;
	xc[2] = lx;
	yc[2] = 0.0;
	xc[3] = lx;
	yc[3] = ly;

	xc[4] = lx / 2;
	yc[4] = 0.0;
	xc[5] = 0.0;
	yc[5] = ly / 2;
	xc[6] = lx;
	yc[6] = ly / 2;
	xc[7] = lx / 2;
	yc[7] = ly;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				tag = 1;
				for (nc = 0; nc < 4; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 4; nc < 8; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
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
void initW_aC62(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[13], yc[13];
	double xi, yj, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aC62.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fCinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

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

	xc[5] = lx / 2;
	yc[5] = 0.0;
	xc[6] = lx / 2;
	yc[6] = ly;
	xc[7] = 0.0;
	yc[7] = ly / 2;
	xc[8] = lx;
	yc[8] = ly / 2;
	xc[9] = lx / 4;
	yc[9] = ly / 4;
	xc[10] = lx / 4;
	yc[10] = ly / 4 * 3;
	xc[11] = lx / 4 * 3;
	yc[11] = ly / 4;
	xc[12] = lx / 4 * 3;
	yc[12] = ly / 4 * 3;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				tag = 1;
				for (nc = 0; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 5; nc < 13; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
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

void initW_aC63(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[11], yc[11];
	double xi, yj, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aC63.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fCinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

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

	xc[5] = lx / 2;
	yc[5] = ly / 6;
	xc[6] = lx / 2;
	yc[6] = ly / 6 * 5;
	xc[7] = 0;
	yc[7] = ly / 3;
	xc[8] = 0;
	yc[8] = ly / 3 * 2;
	xc[9] = lx;
	yc[9] = ly / 3;
	xc[10] = lx;
	yc[10] = ly / 3 * 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				tag = 1;
				for (nc = 0; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 5; nc < 11; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
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

void initW_aC3(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[8], yc[8];
	double xi, yj, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aC3.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fCinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

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

	xc[5] = lx / 2;
	yc[5] = ly / 6;
	xc[6] = 0;
	yc[6] = ly / 3 * 2;
	xc[7] = lx;
	yc[7] = ly / 3 * 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				tag = 1;
				for (nc = 0; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 5; nc < 8; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
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

void initW_csPL(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[9], yc[9], zc[3];
	double xi, yj, zk, rij, lij, rc, la, lb;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csPL.dat", "w");

	rc = pow((fAinit + fBinit) / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

	la = fAinit / (fA + fB + fC) * lx / 4;
	lb = (fAinit + fBinit) / (fA + fB + fC) * lx / 4;

	zc[0] = 0.0;
	zc[1] = lz;
	zc[2] = lz / 2;
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
	xc[5] = 0.0;
	yc[5] = ly / 2;
	xc[6] = lx / 2;
	yc[6] = 0.0;
	xc[7] = lx / 2;
	yc[7] = ly;
	xc[8] = lx;
	yc[8] = ly / 2;

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
				for (nc = 0; nc < 3; nc++)
				{
					zij = zk - zc[nc];
					lij = sqrt(zij * zij);
					if (lij < lb)
					{
						tag = 1;
						if (lij < la)
						{
							tag = 2;
						}
					}
				}
				if ((zk < lz * 3 / 4) && (zk > lz * 1 / 4))
				{
					for (nc = 0; nc < 5; nc++)
					{
						xij = xi - xc[nc];
						yij = yj - yc[nc];

						rij = xij * xij + yij * yij;
						rij = sqrt(rij);
						if (rij < rc)
						{
							tag = 0;
						}
					}
				}
				else
				{
					for (nc = 5; nc < 9; nc++)
					{
						xij = xi - xc[nc];
						yij = yj - yc[nc];

						rij = xij * xij + yij * yij;
						rij = sqrt(rij);
						if (rij < rc)
						{
							tag = 0;
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

void initW_csPL_z(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double zc[9], yc[9], xc[3];
	double xi, yj, zk, rij, lij, rc, la, lb;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csPL_z.dat", "w");

	rc = pow((fAinit + fBinit) / (fA + fB + fC) * lz * ly / (2 * Pi), 1.0 / 2);

	la = fAinit / (fA + fB + fC) * lz / 4;
	lb = (fAinit + fBinit) / (fA + fB + fC) * lz / 4;

	xc[0] = 0.0;
	xc[1] = lx;
	xc[2] = lx / 2;
	zc[0] = 0.0;
	yc[0] = 0.0;
	zc[1] = 0.0;
	yc[1] = ly;
	zc[2] = lz;
	yc[2] = 0.0;
	zc[3] = lz;
	yc[3] = ly;
	zc[4] = lz / 2;
	yc[4] = ly / 2;
	zc[5] = 0.0;
	yc[5] = ly / 2;
	zc[6] = lz / 2;
	yc[6] = 0.0;
	zc[7] = lz / 2;
	yc[7] = ly;
	zc[8] = lz;
	yc[8] = ly / 2;

	for (k = 0; k < Nz; k++)
	{
		zk = k * dz;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (i = 0; i < Nx; i++)
			{
				xi = i * dx;
				tag = 0;
				for (nc = 0; nc < 3; nc++)
				{
					xij = xi - xc[nc];
					lij = sqrt(xij * xij);
					if (lij < lb)
					{
						tag = 1;
						if (lij < la)
						{
							tag = 2;
						}
					}
				}
				if ((xi < lx * 3 / 4) && (xi > lx * 1 / 4))
				{
					for (nc = 0; nc < 5; nc++)
					{
						zij = zk - zc[nc];
						yij = yj - yc[nc];

						rij = zij * zij + yij * yij;
						rij = sqrt(rij);
						if (rij < rc)
						{
							tag = 0;
						}
					}
				}
				else
				{
					for (nc = 5; nc < 9; nc++)
					{
						zij = zk - zc[nc];
						yij = yj - yc[nc];

						rij = zij * zij + yij * yij;
						rij = sqrt(rij);
						if (rij < rc)
						{
							tag = 0;
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

void initW_aLC(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[5], yc[5], zc[2];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_csPS.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (2 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = fCinit / (fA + fB + fC) * lx / 2;

	xc[0] = 0.0;
	yc[0] = 0.0;
	xc[1] = lx;
	yc[1] = 0.0;
	xc[2] = 0.0;
	yc[2] = ly;
	xc[3] = lx;
	yc[3] = ly;
	xc[4] = lx / 2;
	yc[4] = ly / 2;
	zc[0] = 0.0;
	zc[1] = lz;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 1;
				for (nc = 0; nc < 5; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - 0.5;

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
				}
				for (nc = 0; nc < 2; nc++)
				{
					zij = zk - zc[nc];
					rij = sqrt(zij * zij);
					if (rij < rc)
						tag = 0;
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

void initW_aCS63(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[14], yc[14], zc[14];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aCS63.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fCinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

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
	xc[5] = 0.5 * lx;
	yc[5] = 0.16667 * ly;
	zc[5] = 0.5 * lz;
	xc[6] = 0.0 * lx;
	yc[6] = 0.66667 * ly;
	zc[6] = 0.5 * lz;
	xc[7] = 1.0 * lx;
	yc[7] = 0.66667 * ly;
	zc[7] = 0.5 * lz;
	xc[8] = 0.0 * lx;
	yc[8] = 0.33333 * ly;
	zc[8] = 0.0 * lz;
	xc[9] = 1.0 * lx;
	yc[9] = 0.33333 * ly;
	zc[9] = 0.0 * lz;
	xc[10] = 0.5 * lx;
	yc[10] = 0.83333 * ly;
	zc[10] = 0.0 * lz;
	xc[11] = 0.0 * lx;
	yc[11] = 0.33333 * ly;
	zc[11] = 1.0 * lz;
	xc[12] = 1.0 * lx;
	yc[12] = 0.33333 * ly;
	zc[12] = 1.0 * lz;
	xc[13] = 0.5 * lx;
	yc[13] = 0.83333 * ly;
	zc[13] = 1.0 * lz;

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
				}
				for (nc = 5; nc < 14; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rc)
					{
						tag = 2;
					}
				}
				phat = 0.0;
				phbt = 1.0;
				phct = 0.0;

				if (tag == 1)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 0.0;
					phbt = 0.0;
					phct = 1.0;
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

void initW_aSC63(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[14], yc[14], zc[14];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aCS63.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fCinit / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

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
	xc[5] = 0.5 * lx;
	yc[5] = 0.16667 * ly;
	zc[5] = 0.5 * lz;
	xc[6] = 0.0 * lx;
	yc[6] = 0.66667 * ly;
	zc[6] = 0.5 * lz;
	xc[7] = 1.0 * lx;
	yc[7] = 0.66667 * ly;
	zc[7] = 0.5 * lz;
	xc[8] = 0.0 * lx;
	yc[8] = 0.33333 * ly;
	zc[8] = 0.0 * lz;
	xc[9] = 1.0 * lx;
	yc[9] = 0.33333 * ly;
	zc[9] = 0.0 * lz;
	xc[10] = 0.5 * lx;
	yc[10] = 0.83333 * ly;
	zc[10] = 0.0 * lz;
	xc[11] = 0.0 * lx;
	yc[11] = 0.33333 * ly;
	zc[11] = 1.0 * lz;
	xc[12] = 1.0 * lx;
	yc[12] = 0.33333 * ly;
	zc[12] = 1.0 * lz;
	xc[13] = 0.5 * lx;
	yc[13] = 0.83333 * ly;
	zc[13] = 1.0 * lz;

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
					if (rij < rc)
					{
						tag = 2;
					}
				}
				for (nc = 5; nc < 14; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
					{
						tag = 1;
					}
				}
				phat = 0.0;
				phbt = 1.0;
				phct = 0.0;

				if (tag == 1)
				{
					phat = 1.0;
					phbt = 0.0;
					phct = 0.0;
				}
				else if (tag == 2)
				{
					phat = 0.0;
					phbt = 0.0;
					phct = 1.0;
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

void initW_H2C(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[15], yc[15];
	double xi, yj, zk, rij, rb, ra, rc;
	double phat, phbt, phct;
	double theta;
	FILE *fp;
	fp = fopen("init_H2C.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);
	rb = pow(fBinit / (fA + fB + fC) * lx * ly / (4 * Pi) / 2, 1.0 / 2);

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

				for (nc = 5; nc < 9; nc++)
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

				for (nc = 9; nc < 10; nc++)
				{
					xc[nc] = xc[nc - 5] + ra * cos(-theta + Pi / 2);
					yc[nc] = yc[nc - 5] + ra * sin(-theta + Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 10; nc < 14; nc++)
				{
					xc[nc] = xc[nc - 10] + ra * cos(theta - Pi / 2);
					yc[nc] = yc[nc - 10] + ra * sin(theta - Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 14; nc < 15; nc++)
				{
					xc[nc] = xc[nc - 10] + ra * cos(-theta - Pi / 2);
					yc[nc] = yc[nc - 10] + ra * sin(-theta - Pi / 2);

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

void initW_H3C(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[20], yc[20];
	double xi, yj, zk, rij, rb, ra, rc;
	double phat, phbt, phct;
	double theta;
	FILE *fp;
	fp = fopen("init_H3C.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);
	rb = pow(fBinit / (fA + fB + fC) * lx * ly / (4 * Pi) / 3, 1.0 / 2);

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

				for (nc = 5; nc < 9; nc++)
				{
					xc[nc] = xc[nc - 5] + ra * cos(theta);
					yc[nc] = yc[nc - 5] + ra * sin(theta);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 9; nc < 10; nc++)
				{
					xc[nc] = xc[nc - 5] + ra * cos(-theta);
					yc[nc] = yc[nc - 5] + ra * sin(-theta);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 10; nc < 14; nc++)
				{
					xc[nc] = xc[nc - 10] + ra * cos(theta - 2 * Pi / 3);
					yc[nc] = yc[nc - 10] + ra * sin(theta - 2 * Pi / 3);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 14; nc < 15; nc++)
				{
					xc[nc] = xc[nc - 10] + ra * cos(-theta - 2 * Pi / 3);
					yc[nc] = yc[nc - 10] + ra * sin(-theta - 2 * Pi / 3);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 15; nc < 19; nc++)
				{
					xc[nc] = xc[nc - 15] + ra * cos(theta + 2 * Pi / 3);
					yc[nc] = yc[nc - 15] + ra * sin(theta + 2 * Pi / 3);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 19; nc < 20; nc++)
				{
					xc[nc] = xc[nc - 15] + ra * cos(-theta + 2 * Pi / 3);
					yc[nc] = yc[nc - 15] + ra * sin(-theta + 2 * Pi / 3);

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

void initW_H4C(double *wA, double *wB, double *wC)
{
	int i, j, k, nc, tag;
	long ijk;
	double xij, yij;
	double xc[25], yc[25];
	double xi, yj, zk, rij, rb, ra, rc;
	double phat, phbt, phct;
	double theta;
	FILE *fp;
	fp = fopen("init_H4C.dat", "w");

	ra = pow(fAinit / (fA + fB + fC) * lx * ly / (4 * Pi), 1.0 / 2);
	rb = pow(fBinit / (fA + fB + fC) * lx * ly / (4 * Pi) / 4, 1.0 / 2);

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

				for (nc = 5; nc < 9; nc++)
				{
					xc[nc] = xc[nc - 5] + ra * cos(theta);
					yc[nc] = yc[nc - 5] + ra * sin(theta);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 9; nc < 10; nc++)
				{
					xc[nc] = xc[nc - 5] + ra * cos(-theta);
					yc[nc] = yc[nc - 5] + ra * sin(-theta);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 10; nc < 14; nc++)
				{
					xc[nc] = xc[nc - 10] + ra * cos(theta + Pi / 2);
					yc[nc] = yc[nc - 10] + ra * sin(theta + Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 14; nc < 15; nc++)
				{
					xc[nc] = xc[nc - 10] + ra * cos(-theta + Pi / 2);
					yc[nc] = yc[nc - 10] + ra * sin(-theta + Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 15; nc < 19; nc++)
				{
					xc[nc] = xc[nc - 15] + ra * cos(theta - Pi / 2);
					yc[nc] = yc[nc - 15] + ra * sin(theta - Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 19; nc < 20; nc++)
				{
					xc[nc] = xc[nc - 15] + ra * cos(-theta - Pi / 2);
					yc[nc] = yc[nc - 15] + ra * sin(-theta - Pi / 2);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 20; nc < 24; nc++)
				{
					xc[nc] = xc[nc - 20] + ra * cos(theta + Pi);
					yc[nc] = yc[nc - 20] + ra * sin(theta + Pi);

					xij = xi - xc[nc];
					yij = yj - yc[nc];

					rij = xij * xij + yij * yij;
					rij = sqrt(rij);

					if (rij < rb)
						tag = 2;
				}

				for (nc = 24; nc < 25; nc++)
				{
					xc[nc] = xc[nc - 20] + ra * cos(-theta + Pi);
					yc[nc] = yc[nc - 20] + ra * sin(-theta + Pi);

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
	double xc[27], yc[27], zc[27];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aPS.dat", "w");

	ra = pow((fAinit / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);
	rc = pow((fCinit / (fA + fB + fC) * lx * ly * lz / (16 * 4.0 * Pi / 3.0)), 1.0 / 3);

	xc[0] = 0.0;
	yc[0] = 0.0;
	zc[0] = 0.0;
	xc[1] = lx;
	yc[1] = 0.0;
	zc[1] = 0.0;
	xc[2] = 0.0;
	yc[2] = ly;
	zc[2] = 0.0;
	xc[3] = lx;
	yc[3] = ly;
	zc[3] = 0.0;
	xc[4] = lx / 2;
	yc[4] = ly / 2;
	zc[4] = 0.0;
	xc[5] = 0.0;
	yc[5] = 0.0;
	zc[5] = lz;
	xc[6] = lx;
	yc[6] = 0.0;
	zc[6] = lz;
	xc[7] = 0.0;
	yc[7] = ly;
	zc[7] = lz;
	xc[8] = lx;
	yc[8] = ly;
	zc[8] = lz;
	xc[9] = lx / 2;
	yc[9] = ly / 2;
	zc[9] = lz;
	xc[10] = lx / 2;
	yc[10] = 0.0;
	zc[10] = lz / 2;
	xc[11] = 0.0;
	yc[11] = ly / 2;
	zc[11] = lz / 2;
	xc[12] = lx;
	yc[12] = ly / 2;
	zc[12] = lz / 2;
	xc[13] = lx / 2;
	yc[13] = ly;
	zc[13] = lz / 2;

	xc[14] = 0.0;
	yc[14] = ly / 2;
	zc[14] = 0.0;
	xc[15] = lx / 2;
	yc[15] = 0.0;
	zc[15] = 0.0;
	xc[16] = lx;
	yc[16] = ly / 2;
	zc[16] = 0.0;
	xc[17] = lx / 2;
	yc[17] = ly;
	zc[17] = 0.0;
	xc[18] = lx;
	yc[18] = ly / 2;
	zc[18] = lz;
	xc[19] = lx / 2;
	yc[19] = ly;
	zc[19] = lz;
	xc[20] = 0.0;
	yc[20] = ly / 2;
	zc[20] = lz;
	xc[21] = lx / 2;
	yc[21] = 0.0;
	zc[21] = lz;
	xc[22] = 0.0;
	yc[22] = 0.0;
	zc[22] = lz / 2;
	xc[23] = lx;
	yc[23] = 0.0;
	zc[23] = lz / 2;
	xc[24] = 0.0;
	yc[24] = ly;
	zc[24] = lz / 2;
	xc[25] = lx;
	yc[25] = ly;
	zc[25] = lz / 2;
	xc[26] = lx / 2;
	yc[26] = ly / 2;
	zc[26] = lz / 2;

	for (i = 0; i < Nx; i++)
	{
		xi = i * dx;
		for (j = 0; j < Ny; j++)
		{
			yj = j * dy;
			for (k = 0; k < Nz; k++)
			{
				zk = k * dz;
				tag = 1;
				for (nc = 0; nc < 14; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < rc)
						tag = 0;
				}
				for (nc = 14; nc < 27; nc++)
				{
					xij = xi - xc[nc];
					yij = yj - yc[nc];
					zij = zk - zc[nc];

					rij = xij * xij + yij * yij + zij * zij;
					rij = sqrt(rij);
					if (rij < ra)
						tag = 2;
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

	Sm1 = 5.0e-6;
	Sm2 = 0.1e-7;
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

		if (iter == 1 || iter % 100 == 0)
			write_ph(phA, phB, phC, wA, wB, wC);
	} while (iter < maxIter && (inCompMax > Sm1 || freeDiff > Sm2));

	fp = fopen("printout.txt", "a");
	fprintf(fp, "%d\n", iter);
	fprintf(fp, "%10.8e, %10.8e, %10.8e, %10.8e, %e\n", freeEnergy, freeAB, freeW, freeS, inCompMax);
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

double getConc(phlA, phlB, phlC, phs0, wA, wB, wC) 
double *phlA, *phlB, *phlC, phs0;
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
