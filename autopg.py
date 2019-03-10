from numpy import arange

runlist=[]
for i in arange(0.02,0.98,0.02):
    for j in arange(0.02,1-i,0.02):
        k=1-i-j
        runlist.append([i,j,k,])