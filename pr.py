import numpy as np
import matplotlib.pyplot as plt

def setText(text):
    print(text)

def geteig(file,nx,ny,nz):
    '''
    Get the FFT array

    '''

    #Defining size of pha
    # nx = 64
    # ny = 64
    # nz = 64

    #Read pha file
    phafile = open(file, 'r')
    phalines = phafile.readlines()
    pha = [[], [], []]
    sf = [[], [], []]
    for item in phalines:
        if not item == '\n':
            list = item.split(' ')
            pha[0].append(float(list[0]))
            pha[1].append(float(list[1]))
            pha[2].append(float(list[2]))
    phafile.close()

    #Convert the pha to a three dimentional array
    pha = [np.array(item).reshape(nx, ny, nz) for item in pha]

    #The size of Fourier Space divided by size of pha
    d=8

    #DFT
    sf[0] = [np.fft.fft2(item[int(nx/2),::,::],[64*d,64*d]).real[0:int(4*d),0:int(4*d)] for item in pha]
    sf[1] = [np.fft.fft2(item[::,int(ny/2),::],[64*d,64*d]).real[0:int(4*d),0:int(4*d)] for item in pha]
    sf[2] = [np.fft.fft2(item[::,::,int(nz/2)],[64*d,64*d]).real[0:int(4*d),0:int(4*d)] for item in pha]
    #Coordinate of sf: three slices, three components, 2D Fourier Square

    #Eliminate the main peak at (0,0)
    # for item in sf:
        # for subitem in item:
        #     subitem[0][0]=0

    #Judging
    if abs(sf[0][0][0][1] - np.min(sf[0][0])) < 0.1 and abs(sf[0][0][1][0] - np.min(sf[0][0])) < 0.1 and abs(sf[0][0][1][1] - np.max(sf[0][0])) < 0.1:
        setText('BCC\n')
    elif abs(sf[0][0][0][2] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][2][0] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][1][1] - np.min(sf[0][0])) < 0.1:
        setText('FCC\n')
    elif abs(sf[0][0][1][1] - np.min(sf[0][0])) < 0.1 and abs(sf[1][0][0][1] - np.max(sf[1][0])) < 0.1 and abs(sf[2][0][0][1] - np.max(sf[2][0])) < 0.1:
        setText('G\n')
    elif abs(sf[0][0][2][0] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][1][2] - np.min(sf[0][0])) < 0.1 and abs(sf[1][0][0][2] - np.max(sf[1][0])) < 0.1:
        setText('A15\n')
    elif abs(sf[0][0][0][1] - np.min(sf[0][0])) < 0.1 and abs(sf[0][0][1][1] - np.max(sf[0][0])) < 0.1 and abs(sf[1][0][2][0] - np.min(sf[1][0])) < 0.1:
        setText('csHelix\n')
    elif abs(sf[0][0][1][0] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][2][1] - np.min(sf[0][0])) < 0.1 and abs(sf[1][0][2][1] - np.min(sf[1][0])) < 0.1:
        setText('csHelix2\n')
    elif abs(sf[0][0][0][2] - np.max(sf[0][0])) < 0.1 and abs(sf[0][0][2][1] - np.min(sf[0][0])) < 0.1 and abs(sf[1][0][0][2] - np.max(sf[1][0])) < 0.1:
        setText('csσ\n')
    return sf

sf=geteig('d:/SCFT/pha.dat',64,64,64)

#Plot
plt.imshow(sf[0][0])
plt.show()

# import os
# path=os.listdir('d:/nonfrustrated/nonfrustrated/633')
# for item in path:
#     print('633:'+item)
#     geteig('d:/nonfrustrated/nonfrustrated/633/'+item+'/pha.dat',64,64,64)