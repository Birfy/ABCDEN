def geteig(filename,nx=64,ny=64,nz=64):
    '''
    Get a array with three fft arrays of each component
    '''

    # Read pha file
    phafile = open(filename, 'r')
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

    # Convert the pha to a nx*ny*nz matrix
    pha = [np.array(item).reshape(nx, ny, nz) for item in pha]

    # The size of Fourier Space divided by size of pha
    d=2

    # FFT the concentration to a 64*64*64 matrix
    sf = abs(np.fft.fftn(pha[0],[64*d,64*d,64*d]))[0:int(4*d),0:int(4*d),0:int(4*d)]

    del pha
    # sf = [abs(np.fft.fftn(item,[64*d,64*d,64*d]))[0:int(4*d),0:int(4*d),0:int(4*d)] for item in pha]
    # sf[1] = [abs(np.fft.fftn(item,[1,64*d,64*d]))[:,0:int(4*d),0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
    # sf[2] = [abs(np.fft.fftn(item,[64*d,1,64*d]))[0:int(4*d),:,0:int(4*d)].reshape(int(4*d),int(4*d)) for item in pha]
    
    # Convert all the values to [0,1]
    # for item in sf:
            # item/=item[0][0][0]
    sf /= sf[0][0][0]
    return sf

import numpy as np
import matplotlib.pyplot as plt
import os

# Import datasets, classifiers and performance metrics
from sklearn import datasets, svm, metrics

# Import joblib to save the result
from sklearn.externals import joblib

# To apply a classifier on your data, we need to flatten the image
# turn the data in a (samples, feature) matrix:

# The images to learn
images=[]
# The labels of each image to identify which phase it belongs to
target=[]

# Read phas from file and convert them into fft matrixs
path=os.listdir('d:/nonfrustrated/nonfrustrated/633')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/633/'+item+'/pha.dat',64,64,64))
    target.append('FCC')
    print(item)
path=os.listdir('d:/nonfrustrated/nonfrustrated/233')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/233/'+item+'/pha.dat',64,64,64))
    target.append('BCC')
    print(item)
path=os.listdir('d:/nonfrustrated/nonfrustrated/15')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/15/'+item+'/pha.dat',64,64,64))
    target.append('A15')
    print(item)
path=os.listdir('d:/nonfrustrated/nonfrustrated/8')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/8/'+item+'/pha.dat',64,128,48))
    target.append('csHelix')
    print(item)
path=os.listdir('d:/nonfrustrated/nonfrustrated/82')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/82/'+item+'/pha.dat',64,128,48))
    target.append('csHelix2')
    print(item)
path=os.listdir('d:/nonfrustrated/nonfrustrated/0')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/0/'+item+'/pha.dat',64,64,64))
    target.append('Random')
    print(item)

# Number of the samples
n_samples = len(images)

# Reshape the samples into a vector by connecting the lines head by foot
data = np.array(images).reshape((n_samples, -1))

# Create a classifier: a support vector classifier, kernel refers to the kernel function, C is a penalty parameter of the error term, tol is the tolerance for stopping criterion
classifier = svm.SVC(kernel='linear',probability=True)
# estimator=svm.OneClassSVM(kernel='linear',nu=0.001)
from sklearn.ensemble import IsolationForest
estimator=IsolationForest(max_samples=n_samples,contamination=0.04,n_estimators=n_samples)
# from sklearn.neighbors import LocalOutlierFactor
# estimator=LocalOutlierFactor(n_neighbors=15,contamination=0.1)
# from sklearn.covariance import EllipticEnvelope
# estimator=EllipticEnvelope(contamination=0.1)

# Train
classifier.fit(data[:], target[:])
estimator.fit(data[:])

# Save the result to a file
with open('classifier.pkl','wb') as file:
    joblib.dump(classifier, file)

with open('estimator.pkl','wb') as file:
    joblib.dump(estimator, file)

# Now predict the all the values:
expected = target[:]
predicted = estimator.predict(data[:])
for i in range(len(target)):
    print(expected[i],predicted[i])

predicted = classifier.predict(data[:])
for i in range(len(target)):
    print(expected[i],predicted[i])

# Print a classification report
print("Classification report for classifier %s:\n%s\n"
      % (classifier, metrics.classification_report(expected, predicted)))
print("Confusion matrix:\n%s" % metrics.confusion_matrix(expected, predicted))