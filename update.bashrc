scp ./abcden.c cdy@10.158.131.62:/export/home/cdy/update/abcden.c
scp ./para cdy@10.158.131.62:/export/home/cdy/update/para
echo 'UPLOADED'
ssh cdy@10.158.131.62 "cd update; gcc abcden.c -o abcden -lm -lgsl -lgslcblas -lfftw3 -L/export/home/cdy/fftw3/lib"
echo 'COMPILED'