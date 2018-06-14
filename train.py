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

import numpy as np
# Standard scientific Python imports
import matplotlib.pyplot as plt

# Import datasets, classifiers and performance metrics
from sklearn import datasets, svm, metrics
 
# # The digits dataset
# digits = datasets.load_digits()
# # print(len(digits.target))

# # The data that we are interested in is made of 8x8 images of digits, let's
# # have a look at the first 4 images, stored in the `images` attribute of the
# # dataset.  If we were working from image files, we could load them using
# # matplotlib.pyplot.imread.  Note that each image must have the same size. For these
# # images, we know which digit they represent: it is given in the 'target' of
# # the dataset.
# images_and_labels = list(zip(digits.images, digits.target))
# for index, (image, label) in enumerate(images_and_labels[:4]):
#     plt.subplot(2, 4, index + 1)
#     plt.axis('off')
#     plt.imshow(image, cmap=plt.cm.gray_r, interpolation='nearest')
#     plt.title('Training: %i' % label)

# # To apply a classifier on this data, we need to flatten the image, to
# # turn the data in a (samples, feature) matrix:
images=[]
target=[]
import os
path=os.listdir('d:/nonfrustrated/nonfrustrated/633')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/633/'+item+'/pha.dat',64,64,64)[0])
    target.append('633')
    print(item)
path=os.listdir('d:/nonfrustrated/nonfrustrated/233')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/233/'+item+'/pha.dat',64,64,64)[0])
    target.append('233')
    print(item)
path=os.listdir('d:/nonfrustrated/nonfrustrated/15')
for item in path:
    images.append(geteig('d:/nonfrustrated/nonfrustrated/15/'+item+'/pha.dat',64,64,64)[0])
    target.append('15')
    print(item)


n_samples = len(images)
data = np.array(images).reshape((n_samples, -1))

# Create a classifier: a support vector classifier
classifier = svm.SVC(kernel='linear',C=100,tol=1e-5)

# We learn the digits on the first half of the digits
classifier.fit(data[:], target[:])
from sklearn.externals import joblib
pkl_filename='classifier.pkl'
with open(pkl_filename,'wb') as file:
    joblib.dump(classifier, file)

# Now predict the value of the digit on the second half:
expected = target[:]
test = np.array(geteig('d:/SCFT/pha.dat',64,64,64)[0]).reshape((1,-1))
print(classifier.predict(test))
# predicted = classifier.predict(data[:])
# for i in range(len(target)):
#     print(expected[i],predicted[i])

# print("Classification report for classifier %s:\n%s\n"
#       % (classifier, metrics.classification_report(expected, predicted)))
# print("Confusion matrix:\n%s" % metrics.confusion_matrix(expected, predicted))

# images_and_predictions = list(zip(digits.images[n_samples // 2:], predicted))
# for index, (image, prediction) in enumerate(images_and_predictions[:4]):
#     plt.subplot(2, 4, index + 5)
#     plt.axis('off')
#     plt.imshow(image, cmap=plt.cm.gray_r, interpolation='nearest')
#     plt.title('Prediction: %i' % prediction)

# plt.show()