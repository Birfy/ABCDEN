'''
This program transfer AB diblock concentrations to ABC Triblock concentrations for specific initiation
By adding a 0 concentration to B component
'''

fileread=open('gyroid.dat','r')
filewrite=open('pha.dat','w')

phalines=fileread.readlines()
count=0
for item in phalines:
    if not item == '\n':
        if count%2==0 and count//64%2==0 and count//64//64%2==0:
            filewrite.write(item.split(' ')[0]+' 0.0 '+item.split(' ')[1]+' '+item.split(' ')[2]+' 0.0 '+item.split(' ')[3])
        count+=1

fileread.close()
filewrite.close()