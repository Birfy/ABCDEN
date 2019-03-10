def geteig(filename,nx=64,ny=64,nz=64):
    '''
    Get a array with three fft arrays of each component
    '''

    # Read pha file
    pha = []
    sf=[]
    for item in open(filename, 'r'):
        if not item == '\n':
            pha.append(float(item.split(' ')[0]))
    # phafile.close()

    # Convert the pha to a nx*ny*nz matrix
    pha = np.array(pha).reshape(nx, ny, nz)

    # The size of Fourier Space divided by size of pha
    d=2

    # FFT the concentration to a 64*64*64 matrix
    sf = abs(np.fft.fftn(pha,[nx*d,ny*d,nz*d]))[0:int(4*d),0:int(4*d),0:int(4*d)]
    # sf = [abs(np.fft.fftn(item,[64*d,64*d,64*d]))[0:int(4*d),0:int(4*d),0:int(4*d)] for item in pha]
    # sf[1] = [abs(np.fft.fftn(item,[1,64*d,64*d]))[:,0:int(4*d),0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
    # sf[2] = [abs(np.fft.fftn(item,[64*d,1,64*d]))[0:int(4*d),:,0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
    
    # Convert all the values to [0,1]
    # for item in sf:
            # item/=item[0][0][0]
    sf /= sf[0][0][0]
    return sf

from sklearn.externals import joblib
import numpy as np

# Load the saved learning result
with open('classifier.pkl','rb') as file:
    classifier=joblib.load(file)

with open('estimator.pkl','rb') as file:
    estimator=joblib.load(file)

# Load a file and convert to fft matrix
matrix = np.array(geteig('./pha.dat',64,64,64)).reshape((1,-1))

# Print the result
# print(estimator.predict(matrix))
if estimator.predict(matrix)[0] > 0:
    print(classifier.predict(matrix)[0])
    print(classifier.predict_proba(matrix)[0])
    print(str(round(max(classifier.predict_proba(matrix)[0]),3)*100)+'%')
else:
    print('not a known phase')
