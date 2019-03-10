#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main()
{
    int Nx = 64;
    int Ny = 64;
    int Nz = 64;

    double Pi=3.1415926;

    double lx = 3;
    double ly = 5.1;
    double lz = 4;

    double dx=lx/Nx;
    double dy=ly/Ny;
    double dz=lz/Nz;

    double fA = 0.2;
    double fB = 0.28;
    double fC = 0.06;

    int i, j, k, nc, tag;
	long ijk;
	double xij, yij, zij;
	double xc[14], yc[14], zc[14];
	double xi, yj, zk, rij, rc, ra;
	double phat, phbt, phct;
	FILE *fp;
	fp = fopen("init_aCS63.dat", "w");

	ra = pow(fA / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);
	rc = pow(fC / (fA + fB + fC) * lx * ly / (2 * Pi), 1.0 / 2);

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
				zk= k * dz;
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
				fprintf(fp, "%lf %lf %lf\n", phat, phbt, phct);
				
			}
		}
	}
	fclose(fp);
    return 0;
}