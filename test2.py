import numpy as np
from matplotlib import pyplot as plt
from numpy import arange
from matplotlib import gridspec 

gs = gridspec.GridSpec(2, 4, width_ratios=[0.2,1,1,1],height_ratios=[1,0.577]) 

plt.subplot(gs[0])
plt.axis('off')
plt.text(0,0.5,'$BCC$\n$f_A=0.20$')

for i in range(3):
    phafile=open('BCC'+str(i+1)+'.dat','r')
    pha=phafile.readlines()
    a=[]
    b=[]
    c=[]
    for item in pha:
        if not item == '\n':
            list=item.split(' ')
            a.append(float(list[0]))
            b.append(float(list[1]))
            c.append(float(list[2]))
        if len(c)==256:
            break
    phafile.close()

    a=np.array(a).reshape(16,16)
    b=np.array(b).reshape(16,16)
    c=np.array(c).reshape(16,16)

    plt.subplot(gs[i+1])
    
    # plt.contourf(c)
    if i==0:
        plt.contourf(c,arange(0,0.04,0.00715))
    elif i==1:
        plt.contourf(c,arange(0,0.25,0.0425))
    elif i==2:
        plt.contourf(c,arange(0,0.62,0.12))
    # plt.contourf(c,arange(np.min(c),np.max(c)+0.005,0.005))
    plt.axis('off')
    plt.title('$f_C=%.2f$' % (0.02+0.10*i))
    plt.colorbar()

plt.subplot(gs[4])
plt.axis('off')
plt.text(0,0.5,'$C_{6,3}$\n$f_A=0.30$')
for i in range(3):
    phafile=open('C'+str(i+1)+'.dat','r')
    pha=phafile.readlines()
    a=[]
    b=[]
    c=[]
    count=0
    for item in pha:
        if not item == '\n' and count == 0:
            count+=1
            list=item.split(' ')
            a.append(float(list[0]))
            b.append(float(list[1]))
            c.append(float(list[2]))
        elif not item == '\n':
            count-=1
        if len(c)==1024:
            break
    phafile.close()

    a=np.array(a).reshape(32,32)
    b=np.array(b).reshape(32,32)
    c=np.array(c).reshape(32,32)

    plt.subplot(gs[i+5])
    # plt.contourf(c)
    if i==0:
        plt.contourf(c,arange(0,0.045,0.008))
    elif i==1:
        plt.contourf(c,arange(0,0.3,0.05))
    elif i==2:
        plt.contourf(c,arange(0,0.72,0.14))
    
    # plt.contourf(c,arange(np.min(c),np.max(c)+0.005,0.005))
    plt.axis('off')
    # plt.colorbar()
    plt.colorbar()

plt.show()



