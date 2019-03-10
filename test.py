def geteig(filename,nx=64,ny=64,nz=64):
    
    #Read pha file
    phafile = open(filename, 'r')
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
    d=8

    #DFT
    sf[0] = [abs(np.fft.fftn(item,[64*d,64*d,1]))[0:int(4*d),0:int(4*d),:].reshape(int(4*d),int(4*d)) for item in pha]
    # sf[1] = [abs(np.fft.fftn(item,[1,64*d,64*d]))[:,0:int(4*d),0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
    # sf[2] = [abs(np.fft.fftn(item,[64*d,1,64*d]))[0:int(4*d),:,0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
    
    # for item in sf:
    for subitem in sf[0]:
        subitem/=subitem[0][0]
    return sf

from sklearn.externals import joblib
import numpy as np
with open('classifier.pkl','rb') as file:
    classifier=joblib.load(file)

test = np.array(geteig('d:/SCFT/pha.dat',64,64,64)[0]).reshape((1,-1))
print(classifier.predict(test))