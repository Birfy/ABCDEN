from matplotlib import pyplot as plt

with open('freeE/233/a0.020b0.380x3.840-a0.380b0.020x3.840') as bcc:
    result = bcc.readlines()
bcc={}
for item in result:
    bcc[item.strip('\n').split(' ')[0]] = item.strip('\n').split(' ')[1]
origin=bcc.copy()
for key, value in bcc.items():
    bcc[key] = str(float(bcc[key]) - float(bcc[key]))
bccplotx=[float(item) for item in bcc.keys()]
bccploty=[float(item) for item in bcc.values()]
plt.plot(bccplotx,bccploty,label='bcc')

with open('freeE/633/a0.020b0.380x4.800-a0.380b0.020x4.800') as fcc:
    result = fcc.readlines()
fcc={}
for item in result:
    fcc[item.strip('\n').split(' ')[0]] = item.strip('\n').split(' ')[1]
for key, value in fcc.items():
    fcc[key] = str(float(fcc[key]) - float(origin[key]))
fccplotx=[float(item) for item in fcc.keys()]
fccploty=[float(item) for item in fcc.values()]
plt.plot(fccplotx,fccploty,label='fcc')

with open('freeE/15/a0.020b0.380x6.120-a0.380b0.020x6.120') as a15:
    result = a15.readlines()
a15={}
for item in result:
    a15[item.strip('\n').split(' ')[0]] = item.strip('\n').split(' ')[1]
for key, value in a15.items():
    a15[key] = str(float(a15[key]) - float(origin[key]))
a15plotx=[float(item) for item in a15.keys()]
a15ploty=[float(item) for item in a15.values()]
plt.plot(a15plotx,a15ploty,label='a15')

with open('freeE/1619/a0.020b0.380x2.087-a0.380b0.020x2.087') as ps:
    result = ps.readlines()
ps={}
for item in result:
    ps[item.strip('\n').split(' ')[0]] = item.strip('\n').split(' ')[1]
for key, value in ps.items():
    ps[key] = str(float(ps[key]) - float(origin[key]))
psplotx=[float(item) for item in ps.keys()]
psploty=[float(item) for item in ps.values()]
plt.plot(psplotx,psploty,label='ps')

with open('freeE/6/a0.100b0.280x11.730-a0.300b0.080x11.730') as sigma:
    result = sigma.readlines()
sigma={}
for item in result:
    sigma[item.strip('\n').split(' ')[0]] = item.strip('\n').split(' ')[1]
for key, value in sigma.items():
    sigma[key] = str(float(sigma[key]) - float(origin[key]))
sigmaplotx=[float(item) for item in sigma.keys()]
sigmaploty=[float(item) for item in sigma.values()]
plt.plot(sigmaplotx,sigmaploty,label='sigma')

plt.legend()
plt.show()