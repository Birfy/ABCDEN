import numpy as np
import matplotlib.pyplot as plt

def setText(text):
    print(text)

#Defining size of pha
nx = 64
ny = 64
nz = 64

#Read pha file
phafile = open('pha.dat', 'r')
phalines = phafile.readlines()
pha = [[], [], []]
sf = []
for item in phalines:
    if not item == '\n':
        list = item.split(' ')
        pha[0].append(float(list[0]))
        pha[1].append(float(list[1]))
        pha[2].append(float(list[2]))
phafile.close()

pha = [np.array(item).reshape(nx, ny, nz) for item in pha]

#The size of Fourier Space divided by size of pha
d=1

#DFT
sf = [abs(np.fft.fftn(item,[64*d,64*d,64*d]))[0:int(4*d),0:int(4*d),0:int(4*d)] for item in pha]
#Coordinate of sf: three components, 3D Fourier Cubic

#Eliminate the main peak at (0,0)
for item in sf:
    item[0][0][0]=0

#Judging
if abs(sf[0][0][1][1] - np.max(sf[0])) < 0.1 and abs(sf[0][1][1][0] - np.max(sf[0])) < 0.1 and abs(sf[0][1][0][1] - np.max(sf[0])) < 0.1:
    setText('BCC\n')
# elif abs(sf[0][0][0][2] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][2][0] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][1][1] - np.min(sf[0][0])) < 0.1:
#     setText('FCC\n')
# elif abs(sf[0][0][1][1] - np.min(sf[0][0])) < 0.1 and abs(sf[1][0][0][1] - np.max(sf[1][0])) < 0.1 and abs(sf[2][0][0][1] - np.max(sf[2][0])) < 0.1:
#     setText('G\n')
# elif abs(sf[0][0][2][0] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][1][2] - np.min(sf[0][0])) < 0.1 and abs(sf[1][0][0][2] - np.max(sf[1][0])) < 0.1:
#     setText('A15\n')
# elif abs(sf[0][0][0][1] - np.min(sf[0][0])) < 0.1 and abs(sf[0][0][1][1] - np.max(sf[0][0])) < 0.1 and abs(sf[1][0][2][0] - np.min(sf[1][0])) < 0.1:
#     setText('csHelix\n')
# elif abs(sf[0][0][1][0] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][2][1] - np.min(sf[0][0])) < 0.1 and abs(sf[1][0][2][1] - np.min(sf[1][0])) < 0.1:
#     setText('csHelix2\n')
elif abs(sf[0][0][2][0] - np.max(sf[0])) < 0.1 and abs(sf[0][2][0][0] - np.max(sf[0])) < 0.1 and abs(sf[1][0][1][2] - np.max(sf[1])) < 0.1:
    setText('csσ\n')

#Plot
# for i in rangesi,j,k,sf[1][i][j][k])
