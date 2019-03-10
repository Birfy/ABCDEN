import numpy as np
import matplotlib.pyplot as plt

def setText(text):
    print(text)

#Defining size of pha
nx = 32
ny = 32
nz = 2

#Read pha file
phafile = open('pha.dat', 'r')
phalines = phafile.readlines()
pha = [[], [], []]
sf = [[],[],[]]
for item in phalines:
    if not item == '\n':
        list = item.split(' ')
        pha[0].append(float(list[0]))
        pha[1].append(float(list[1]))
        pha[2].append(float(list[2]))
phafile.close()

pha = [np.array(item).reshape(nx, ny, nz) for item in pha]

#The size of Fourier Space divided by size of pha
d=16

#DFT
sf[0] = [abs(np.fft.fftn(item,[64*d,64*d,1]))[0:int(4*d),0:int(4*d),:].reshape(int(4*d),int(4*d)) for item in pha]
sf[1] = [abs(np.fft.fftn(item,[1,64*d,64*d]))[:,0:int(4*d),0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
sf[2] = [abs(np.fft.fftn(item,[64*d,1,64*d]))[0:int(4*d),:,0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
#Coordinate of sf: three components, 3D Fourier Cubic

for item in sf:
    for subitem in item:
        subitem/=subitem[0][0]
#Eliminate the main peak at (0,0,0)
# for item in sf:
#     for subitem in item:
#         subitem[0][0]=0

#Judging
# if abs(sf[0][0][1][1] - np.min(sf[0])) < 0.1 and abs(sf[0][1][1][0] - np.max(sf[0])) < 0.1 and abs(sf[1][2][0][2] - np.max(sf[1])) < 0.1:
#     setText('G\n')
# elif abs(sf[0][0][1][1] - np.max(sf[0])) < 0.1 and abs(sf[0][1][1][0] - np.max(sf[0])) < 0.1 and abs(sf[0][1][0][1] - np.max(sf[0])) < 0.1:
#     setText('BCC\n')
# elif abs(sf[0][1][1][1] - np.max(sf[0])) < 0.1 and abs(sf[1][2][0][2] - np.min(sf[1])) < 0.1 and abs(sf[1][2][2][0] - np.min(sf[1])) < 0.1:
#     setText('FCC\n')
# elif abs(sf[0][0][0][2] - np.max(sf[0])) < 0.1 and abs(sf[0][0][2][0] - np.max(sf[0])) < 0.1 and abs(sf[0][2][0][0] - np.max(sf[0])) < 0.1:
#     setText('A15\n')
# elif abs(sf[0][1][0][1] - np.max(sf[0])) < 0.1 and abs(sf[0][0][1][2] - np.min(sf[0])) < 0.1 and abs(sf[1][0][0][1] - np.min(sf[1])) < 0.1:
#     setText('csHelix\n')
# elif abs(sf[0][2][1][1] - np.min(sf[0])) < 0.1 and abs(sf[0][2][1][0] - np.max(sf[0])) < 0.1 and abs(sf[1][0][0][1] - np.min(sf[1])) < 0.1:
#     setText('csHelix2\n')
# elif abs(sf[0][0][0][2] - np.max(sf[0])) < 0.1 and abs(sf[0][0][2][1] - np.min(sf[0])) < 0.1 and abs(sf[0][2][0][1] - np.min(sf[0])) < 0.1:
#     setText('csσ\n')

#Plot
# for i in range(4):
#     for j in range(4):
#         for k in range(4):
#             print(i,j,k,sf[0][i][j])
plt.imshow(sf[0][0])
plt.show()
plt.imshow(sf[1][0])
plt.show()
plt.imshow(sf[2][0])
plt.show()
