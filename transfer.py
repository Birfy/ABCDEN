fileread=open('pha.dat','r')
filewrite=open('fddd.dat','w')

phalines=fileread.readlines()
for item in phalines:
    if not item == '\n':
        filewrite.write(item.split(' ')[0]+' 0.0 '+item.split(' ')[1]+' '+item.split(' ')[2]+' 0.0 '+item.split(' ')[3])

fileread.close()
filewrite.close()