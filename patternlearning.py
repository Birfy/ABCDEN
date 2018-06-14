import numpy as np
import matplotlib.pyplot as plt

def geteig(filename,nx=64,ny=64,nz=64):

    #Read pha file
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

    pha = [np.array(item).reshape(nx, ny, nz) for item in pha]

    #The size of Fourier Space divided by size of pha
    d=1

    #DFT
    sf = [abs(np.fft.fftn(item,[64*d,64*d,64*d]))[0:int(4*d),0:int(4*d),0:int(4*d)] for item in pha]
    #Coordinate of sf: three components, 3D Fourier Cubic

    #Eliminate the main peak at (0,0,0)
    for item in sf:
        item[0][0][0]=0

    #Judging
    if abs(sf[0][0][1][1] - np.min(sf[0])) < 0.1 and abs(sf[0][1][1][0] - np.max(sf[0])) < 0.1 and abs(sf[1][2][0][2] - np.max(sf[1])) < 0.1:
        return 'G'
    elif abs(sf[0][0][1][1] - np.max(sf[0])) < 0.1 and abs(sf[0][1][1][0] - np.max(sf[0])) < 0.1 and abs(sf[0][1][0][1] - np.max(sf[0])) < 0.1:
        return 'BCC'
    elif abs(sf[0][1][1][1] - np.max(sf[0])) < 0.1 and abs(sf[1][2][0][2] - np.min(sf[1])) < 0.1 and abs(sf[1][2][2][0] - np.min(sf[1])) < 0.1:
        return 'FCC'
    elif abs(sf[0][0][0][2] - np.max(sf[0])) < 0.1 and abs(sf[0][0][2][0] - np.max(sf[0])) < 0.1 and abs(sf[0][2][0][0] - np.max(sf[0])) < 0.1:
        return 'A15'
    elif abs(sf[0][1][0][1] - np.max(sf[0])) < 0.1 and abs(sf[0][0][1][2] - np.min(sf[0])) < 0.1 and abs(sf[1][0][0][1] - np.min(sf[1])) < 0.1:
        return 'csHelix'
    elif abs(sf[0][2][1][1] - np.min(sf[0])) < 0.1 and abs(sf[0][2][1][0] - np.max(sf[0])) < 0.1 and abs(sf[1][0][0][1] - np.min(sf[1])) < 0.1:
        return 'csHelix2'
    elif abs(sf[0][0][0][2] - np.max(sf[0])) < 0.1 and abs(sf[0][0][2][1] - np.min(sf[0])) < 0.1 and abs(sf[0][2][0][1] - np.min(sf[0])) < 0.1:
        return 'csσ'

import os
path=os.listdir('d:/nonfrustrated/nonfrustrated/8')
for item in path:
    print('8:'+item)
    print(geteig('d:/nonfrustrated/nonfrustrated/8/'+item+'/pha.dat',64,128,48))
# path=os.listdir('d:/nonfrustrated/nonfrustrated/633')
# for item in path:
#     print('633:'+item)
#     print(geteig('d:/nonfrustrated/nonfrustrated/633/'+item+'/pha.dat',64,64,64))
    


# X=np.array(X)
# print(X)
 
# from sklearn.cluster import MeanShift, estimate_bandwidth

# bandwidth = estimate_bandwidth(X, quantile=0.28)
# ms = MeanShift(bandwidth=bandwidth, bin_seeding=True)
# ms.fit(X)
# labels = ms.labels_
# cluster_centers=ms.cluster_centers_

# labels_unique = np.unique(labels)
# n_clusters_ = len(labels_unique)

# print("number of estimated clusters: %d" % n_clusters_)

# import matplotlib.pyplot as plt
# from itertools import cycle

# plt.figure(1)
# plt.clf()

# colors = cycle('bgrcmykbgrcmykbgrcmykbgrcmyk')
# for k, col in zip(range(n_clusters_), colors):
#     my_members = labels == k
#     cluster_center = cluster_centers[k]
#     plt.plot(X[my_members, 0], X[my_members, 1], X[my_members, 2], col + '.')
#     plt.plot(cluster_center[0], cluster_center[1], cluster_center[2], 'o', markerfacecolor=col,
#              markeredgecolor='k', markersize=14)
# plt.title('Estimated number of clusters: %d' % n_clusters_)
# plt.show()
