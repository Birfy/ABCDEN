scp ./abcden.c cdy@10.158.131.62:./update/abcden.c
echo 'UPLOADED'
ssh cdy@10.158.131.62 "cd update; gcc abcden.c -o abcden -lm -lgsl -lgslcblas -lfftw3 -L/export/home/cdy/fftw3/lib"
echo 'COMPILED'