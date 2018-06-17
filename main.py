'''
ABCDEN SCFT GUI PROGRAM
Package needed:
    Matlab: Matlab API for Python
    Multirun: numpy
    SSH: paramiko
    Mayavi: mayavi, VTK
'''

from tkinter import *
from numpy import arange
import numpy as np
import os
import threading
import paramiko

def setText(text):
    '''
    Add text to the TextView
    '''
    resultText.insert(END,text)
    resultText.see(END)

def loadmatlab():
    '''
    Begin to load Matlab
    Matlab button added when finish loading
    '''
    global eng
    import matlab.engine
    # setText('LOADINGMATLAB\n')
    eng=matlab.engine.start_matlab()
    btmatlab.grid(row=1,column=5,padx=5,pady=5) #Button is added when finish loading matlab
    setText('MATLAB IS LOADED\n')

def generate(fA='',fB='',fC='',lx=0.0):
    '''
    Read paras from Entries and write to para file
    If fA, fB, fC or lx is substituted if not empty
    '''
    global result
    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    if fA:
        result[12]=fA
        result[13]=fB
        result[14]=fC

    if lx:
        result[15]="%.3f" % (lx*float(result[15]))
        result[16]="%.3f" % (lx*float(result[16]))
        result[17]="%.3f" % (lx*float(result[17]))

    para = open('para','w',newline='\n')
    columns=[1,2,2,1,2,1,3,3,3,3,1,1,1,3,1]
    index=0
    for item in columns:
        for i in range(item):
            para.write(result[index])
            if i<(item-1):
                para.writelines(', ')
            else:
                para.writelines('\n')
            index+=1
    para.close()
    setText("PARA GENERATED\n")

def sshcommand(cmd):
    '''
    Proceed a ssh command and get result
    '''
    stdin, stdout, stderr = ssh.exec_command(cmd)
    message = stdout.read()
    if not message:
        message = stderr.read()
    return message.decode()

def printout():
    '''
    Get the printout result from path
    '''
    def getprintout():
        setText('GETTING PRINTOUT\n')
        result=[]
        for i in range(len(labels)):
            result.append(entry[i].get().strip())
        message = sshcommand("rsh c0103 ' cat "+result[-1]+"/printout.txt '")
        setText(message+'\n')
    t=threading.Thread(target=getprintout)
    t.start()

def autoremove():
    '''
    Kill the selected progress
    Remove the selected direction
    '''
    def selectremove():
        id = [item.strip(' ').split(' ')[0] for item in sshcommand("rsh " + nodeVar.get() + " 'ps -A w|grep 'a0\.[0-9]*b0\.[0-9]*x[0-9]\.[0-9]*''").strip('\n').split('\n')]
        name = [item.strip(' ').split(' ')[-1] for item in sshcommand("rsh " + nodeVar.get() + " 'ps -A w|grep 'a0\.[0-9]*b0\.[0-9]*x[0-9]\.[0-9]*''").strip('\n').split('\n')]
        remove=Tk()
        remove.title("Autoremove")
        remove.resizable(width=False, height=False)
        var=[]

        if name:
            for j,i in enumerate(name):
                var.append(IntVar(remove))
                Checkbutton(remove, text=i, variable=var[j]).grid(row=j//2,column=j%2)

        def autoremove_kill():
            for j,i in enumerate(var):
                if i.get():
                    sshcommand("rsh " + nodeVar.get() + " 'kill "+id[j]+"'")
                    setText(id[j]+' KILLED\n')

        def autoremove_remove():
            name_init=[]
            name_name=[]
        
            for i in name:
                name_init.append(i.split('a')[0].strip('./'))
                name_name.append('a'+i.split('a')[-1])
                
            result=[]
            for i in range(len(labels)):
                result.append(entry[i].get().strip())
            
            for j,i in enumerate(var):
                if i.get():
                    sshcommand("rsh " + nodeVar.get() + " 'rm -rf "+result[-1]+'/'+name_init[j]+'/'+name_name[j]+"'")
                    setText(result[-1]+'/'+name_init[j]+'/'+name_name[j]+' REMOVED\n')

        Button(remove, text='Kill', command=autoremove_kill, width=20).grid(row=j, column=0 , pady=4)
        Button(remove, text='Remove', command=autoremove_remove, width=20).grid(row=j, column=1, pady=4)

        remove.mainloop()
    t=threading.Thread(target=selectremove)
    t.start()



def update():
    '''
    Upload para file to ./update in Linux
    '''
    def updateonline():
        setText('UPDATING\n')
        sftp.put('./abcden.c', './update/abcden.c')
        sshcommand("cd update; gcc abcden.c -o abcden -lm -lgsl -lgslcblas -lfftw3 -L/export/home/cdy/fftw3/lib;cd")
        setText('COMPILED\n')
    
    t=threading.Thread(target=updateonline)
    t.start()
    

def run(path='',fA='',fB='',fC='',lx=0.0):
    '''
    Copy the abcden file from ./update to path and para file to path in Server
    Run the abcden file on path
    '''
    def runonline():
        result=[]
        for i in range(len(labels)):
            result.append(entry[i].get().strip())

        filepath=path
        setText('RUNNINGONLINE\n')

        generate(fA,fB,fC,lx)

        if not filepath:
            filepath = result[len(result) - 1]
            filename='/'+result[0]+'a'+"%.3f" % float(result[12])+'b'+"%.3f" % float(result[13])+"x"+"%.3f" % float(result[15])
        else:
            filename='/'+filepath.split('/')[-2]+filepath.split('/')[-1]
        
        sshcommand("mkdir " + filepath)
        sftp.put('./para', filepath+"/para")
        sshcommand("cp ./update/abcden " + filepath + filename + ";rsh " + nodeVar.get() + " 'cd " + filepath + ";nohup ."+filename+" >/dev/null 2>&1 &'")
        setText("ABCDEN IS RUNNING\n")

    if path:
        runonline()
    else:
        t=threading.Thread(target=runonline)
        t.start()

def downloadresult(pathlist):
    '''
    Download the freeE in printout.txt from paths in pathlist and save to a file in ./freeE/
    File name is generated according to fA, fB, fC and lx
    '''
    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    def downloadresultonline():
        writefile = open("./freeE/"+result[0]+'/'+pathlist[0].split('/')[-1]+'-'+pathlist[-1].split('/')[-1],'w',newline='\n')
        for i in pathlist:
            setText("DOWNLOADING\n")

            message=sshcommand("cat "+i+"/printout.txt|tail -1|cut -c 1-14")
            writefile.write(message)

        writefile.close()
        setText("FREEDOWNLOADED\n")
    t=threading.Thread(target=downloadresultonline)
    t.start()


def openmultirun():
    '''
    Open a new window
    Check the radiobutton to assign a constant f
    Input the range of the first f besided the constant f
    The rest will be automatically calculated
    Range: begin,end,gap
    '''
    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    mrroot = Tk()
    mrroot.resizable(width=False,height=False)
    mrroot.title('Multirun')

    mrframe=Frame(mrroot)
    v=IntVar(mrroot)
    v.set(0)

    Radiobutton(mrframe,text='fA',variable=v,value=0).grid(row=0,column=0,padx=5)
    Radiobutton(mrframe,text='fB',variable=v,value=1).grid(row=0,column=1,padx=5)
    Radiobutton(mrframe,text='fC',variable=v,value=2).grid(row=0,column=2,padx=5)

    entryfa=Entry(mrframe)
    entryfa.grid(row=1,column=0)
    entryfb=Entry(mrframe)
    entryfb.grid(row=1,column=1)
    entryfc=Entry(mrframe)
    entryfc.grid(row=1,column=2)

    def multirun():
        '''
        Run abcdens in the range
        '''
        def multirunonline():
            fa=entryfa.get().strip()
            fb=entryfb.get().strip()
            fc=entryfc.get().strip()

            if v.get()==0:
                fb=eval('list(arange('+fb+'))')
                for i in fb:
                    run(path=result[-1]+'/'+result[0]+'/a'+"%.3f" % float(fa)+'b'+"%.3f" % i+'x'+result[15],fA="%.3f" % float(fa),fB="%.3f" % i,fC="%.3f" % (1-float(fa)-i))
            elif v.get()==1:
                fa=eval('list(arange('+fa+'))')
                for i in fa:
                    run(path=result[-1]+'/'+result[0]+'/a'+"%.3f" % i+'b'+"%.3f" % float(fb)+'x'+result[15],fA="%.3f" % i,fB="%.3f" % float(fb),fC="%.3f" % (1-i-float(fb)))
            elif v.get()==2:
                fa=eval('list(arange('+fa+'))')
                for i in fa:
                    run(path=result[-1]+'/'+result[0]+'/a'+"%.3f" % i+'b'+"%.3f" % (1-i-float(fc))+'x'+result[15],fA="%.3f" % i,fB="%.3f" % (1-i-float(fc)),fC="%.3f" % float(fc))

        t=threading.Thread(target=multirunonline)
        t.start()

    def getresult():
        '''
        Get the freeE result in the range
        '''
        fa=entryfa.get().strip()
        fb=entryfb.get().strip()
        fc=entryfc.get().strip()
        
        pathlist=[]
        if v.get()==0:
            fb=eval('list(arange('+fb+'))')
            for i in fb:
                pathlist.append(result[-1]+'/'+result[0]+'/a'+"%.3f" % float(fa)+'b'+"%.3f" % i+'x'+result[15])
        elif v.get()==1:
            fa=eval('list(arange('+fa+'))')
            for i in fa:
                pathlist.append(result[-1]+'/'+result[0]+'/a'+"%.3f" % i+'b'+"%.3f" % float(fb)+'x'+result[15])
        elif v.get()==2:
            fa=eval('list(arange('+fa+'))')
            for i in fa:
                pathlist.append(result[-1]+'/'+result[0]+'/a'+"%.3f" % i+'b'+"%.3f" % (1-i-float(fc))+'x'+result[15])
        downloadresult(pathlist)

    btframe=Frame(mrroot)
    Button(btframe,text='run',command=multirun,width=10).grid(row=0,column=0,padx=5)
    Button(btframe,text='getresult',command=getresult,width=10).grid(row=0,column=1,padx=5)

    mrframe.pack()
    btframe.pack()
    mrroot.mainloop()

def openmultilx():
    '''
    Open a new window
    Input the range of lx/lx0
    Range: begin,end,gap
    '''
    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    mrroot = Tk()
    mrroot.resizable(width=False,height=False)
    mrroot.title('Multilx')

    mrframe=Frame(mrroot)
    Label(mrframe,text='lx/lx0').grid(row=0,column=0,padx=5)

    entrylx=Entry(mrframe)
    entrylx.grid(row=0,column=1,padx=5)

    def multilx():
        '''
        Run abcdens in the range
        '''
        def multilxonline():
            lx=entrylx.get().strip()

            lx=eval('list(arange('+lx+'))')
            for i in lx:
                run(path=result[-1]+'/'+result[0]+'/a'+"%.3f" % float(result[12])+'b'+"%.3f" % float(result[13])+"x"+"%.3f" % (i*float(result[15])),lx=i)

        t=threading.Thread(target=multilxonline)
        t.start()

    def getresult():
        '''
        Get the freeE results in the range
        '''
        lx=entrylx.get().strip()
        
        pathlist=[]
        lx=eval('list(arange('+lx+'))')
        for i in lx:
            pathlist.append(result[-1]+'/'+result[0]+'/a'+"%.3f" % float(result[12])+'b'+"%.3f" % float(result[13])+'x'+"%.3f" % (i*float(result[15])))
        downloadresult(pathlist)

    btframe=Frame(mrroot)
    Button(btframe,text='run',command=multilx,width=10).grid(row=0,column=0,padx=5)
    Button(btframe,text='getresult',command=getresult,width=10).grid(row=0,column=1,padx=5)

    mrframe.pack()
    btframe.pack()
    mrroot.mainloop()

def getfile():
    '''
    Down the pha.dat file from path
    '''
    def download():
        setText('DOWNLOADING\n')
        result=[]
        for i in range(len(labels)):
            result.append(entry[i].get().strip())

        sftp.get(result[len(result)-1]+"/pha.dat","./pha.dat")
        setText("FILE DOWNLOADED\n")

    t=threading.Thread(target=download)
    t.start()

def mayavi():
    '''
    Using Mayavi to plot
    '''
    from mayavi import mlab

    setText('PLOTTING\n')

    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    phafile=open('pha.dat','r')
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
    phafile.close()

    n=1

    a=np.array(a).reshape(int(result[18]),int(result[19]),int(result[20]))
    b=np.array(b).reshape(int(result[18]),int(result[19]),int(result[20]))
    c=np.array(c).reshape(int(result[18]),int(result[19]),int(result[20]))

    amatrix=np.zeros((int(result[18])*n,int(result[19])*n,int(result[20])*n))
    bmatrix=np.zeros((int(result[18])*n,int(result[19])*n,int(result[20])*n))
    cmatrix=np.zeros((int(result[18])*n,int(result[19])*n,int(result[20])*n))
    for i in range(int(result[18])*n):
        for j in range(int(result[19])*n):
            for k in range(int(result[20])*n):
                amatrix[i][j][k]=a[i%int(result[18])][j%int(result[19])][k%int(result[20])]
                bmatrix[i][j][k]=b[i%int(result[18])][j%int(result[19])][k%int(result[20])]
                cmatrix[i][j][k]=c[i%int(result[18])][j%int(result[19])][k%int(result[20])]

    src = mlab.pipeline.scalar_field(cmatrix)
    mlab.pipeline.iso_surface(src, opacity=1,contours=[0.5],color=(1,0,0))

    src = mlab.pipeline.scalar_field(bmatrix)
    mlab.pipeline.iso_surface(src, opacity=1,contours=[0.5],color=(0,1,0))

    src = mlab.pipeline.scalar_field(amatrix)
    mlab.pipeline.iso_surface(src, opacity=1,contours=[0.5],color=(0,0,1))

def matlab():
    '''
    Using matlab to plot
    '''
    def plot():
        setText('PLOTTING\n')
        result=[]
        for i in range(len(labels)):
            result.append(entry[i].get().strip())

        eng.workspace['Nx']=float(result[18])
        eng.workspace['Ny']=float(result[19])
        eng.workspace['Nz']=float(result[20])

        eng.plotphabc(nargout=0)   
    
    t=threading.Thread(target=plot)
    t.start()

def top():
    '''
    Get the running abden typed file 
    '''
    def gettop():
        setText('TOPPING\n')
        message = sshcommand("rsh " + nodeVar.get() + " 'ps -A w|grep 'a0\.[0-9]*b0\.[0-9]*x[0-9]\.[0-9]*''")
        setText(message+'\n')
    t=threading.Thread(target=gettop)
    t.start()

def entryCommand(command):
    '''
    Command
    '''
    def putcommand():
        command=cmdEntry.get().strip()
        if command == 'whoislwh':
            lwh=Tk()
            lwh.title('lwh')
            lwhText=Text(lwh,height=57,width=85)
            lwhText.insert(END,\
"kkkc...;k000kkkkOO0O0d,';::ooloooooooodk;...':loxkxxxk0k'..ldl,;:odxO00k;''';l'',olcc\n\
kx;...:xOOOkxkkkxkOkOkdc;::colllllolloo;....cddxxxxxdxOk,..ldc,,coooxxkkkl,'.''',olcl\n\
o,..'cddkkxxdxkxddddxkxdl:::,,cloodddc'.....,odxxxdddxkO,..cocccoooodddoxkOo,''.'cllo\n\
...'coodddxxdxxkkdoooxo;:lc:;,;oddxd:';:dd,''cddxxdlodkO,..:cccooooodoloodkOOc'''';co\n\
;;:ccoodxxkkxkkxddoclko,;:ddl::cdO000XNWWWWWXKK0OkxlloxO;..',;;;;,,;:c:cccldko''''',;\n\
lloocdkOOOOOOOOkxxddxkoc::dxxod0NNWWWWWWWWWWWWWWWWWNKOkO:..',;c:;;;:::cllooxOo'';l,''\n\
oodkOOO00O0OO00000kkkxxxdk00XNNWWMWWWWWWWWWWWWWWWWWWWWWNOo;,,lddooddxxdxxxxkOo'';kOl,\n\
OO00OO0OOO00OO00KK0000KKNNNWWWWWWMWWWWMWWWWWWWWWWWWWWWWWWWNOodddccoddddddddkOo'';ok0O\n\
00KK00000OOOOOO000KXNNWWWWWWMMWWWWWWWWWWMWWWWWWMWWWWWWWWWWWWNXOxlcoddddxddxkOl'';odkd\n\
KKKKK0KKK00OO000KNWWWWWMWWMWMMWWWMWWWMWWMWWWWWWWWWWWWWWWWWWWNNNXkkkddddxddxkOl';cxxl,\n\
KKKKKK00000OO00XNWWWWMMWWWWWMWWWWWWWWWWWWWWWWWWWWWWWWWWWNNWWNNNNNX0kxkkddxxxOdxkO0Okx\n\
KKKKKKK00000KKKNWMWWWWWWWWWWWWWWWWWWWWWNNWWWWWNNWWWNNNNNNNNNNNNNNNNXOOxodxxO00KKKOxOK\n\
0KKKKKK000KXXXNWWMWWWWWWWWWWWWWWNWWWWNXNXXNNNNNXNWNNNXXXXXNNNNNNNNXX0kkxxk0KXXKKK0kkX\n\
KKK0KKKK000KXWWWWWWWWWWWWWWWNNNNNNWNNKKX0KXXKXXKXNXXXXKKKKXXXNNXNNXXXK0OO0KKXXXKKK00K\n\
KKXXXXXXK00KNWWWWWWWWWWWWWWNXNNNXXNXX0O0OO00OOKK0KK0KKOO000KKXXNNXNXXXXXKKXXXXXKKKXXK\n\
0KXXKXKKK00XWWWWWWWWWWWWNNNXKXXX00XK0OOOkxkxodxOxOOO0OdxkOO0O0KXNNNNXXX0KKXXXXXKKXKXK\n\
KKKKKKXKK0KXWWWWWWWWWWWNXXX00K00Ok0kkkkxxodlclldodxkkdoldxxkkkOKNWNNXNNXXKKKXXNXXXXKK\n\
o0KKKKKKKKKXNWWWWNWWWWWXKKKO00Okddxdooollcc:::cllllolccccclooox0NWWNNNNXKKKXXXXNXXXXK\n\
;xOOKXKKKO0XNWWWWWWMMWWX00Okkkxdolll::::;;,,,;;::;;;;;;;;:ccllokXNWWNWNK0XX0XK00XKKXX\n\
ocdolcdx0xl0NMMMWMMMMWN0kkkkkxxxdollcc:;;;,',,',;;,;;::::clllcloKNNNNWNKkXXKX00000xOK\n\
;,ddlcodO:cKWMMMWMMMWN0OkkkOkkkkOOOOkxdolcc:;;;:ccloxxxxxdooooll0NNNNNNXXXXXKkd0KKOdd\n\
..;ll:::dcxXWWWWWWWWWKOkkkkkkxxdolcccloodolc:::cllooollccllllllckWNNNNNXXXK0Okk00K000\n\
,'.';'c;coxKK0KKKNWWN0Okxxxxxxxxxkkkkdoooddl:;:coodxOOOkddlcccloxNNKKNXXNNK00KXK00KXK\n\
:::;;l0kxxkK0O0OO0NWX0OOOOOdxxkkkxxxddolokkOdolxoloxxkkxoddlcclxkXXOOXXXXNKKXNNNWNNXX\n\
oooodk00KKXNNOOkk0NNXOkxxxxdddooooolllclxkkdc;;oxocloolllccccc:llKXxONNNNNXXNWWWWWNNN\n\
0000KKOO0KXNN0kkOKNNXOkkxddoollccccc::cloddlc;,:lc::::::;;::::::cO0kXNNNNXNNWWWWNNWWW\n\
000000O0O0000OkkOO0NX0kkxdooolcccccccloddddl:;,;;:c:::::;;;;,,;;cdodKXXXKKXK00KKKKXXN\n\
kkOOOOOkOOOOOkxxxkO0XKOkxdolcc:;;;;:cloxxdol:;,;;:::;;;;,,'',,;;:::dkkOOxl;;xOOOO0O00\n\
::cccc:;:ccccc:oxxxO00Okxxdlcc::;::cloxddool:;',;;:::::;;,,,,;;:;;:oddddoc''loododddd\n\
cccccc::ccc:;;,llddxkOkkxxdolccccclooxOxxxxdl:;;:c::::llc:;;;;::;:looooodkxoodddddddx\n\
dddddooooooool;oooddxkkkxxxdolllllloodxk0KK0kdlloolc::ccll:::::coooooddddkdoodddddodx\n\
xddddooooooooolllllooxOkkxxdolloooddxxdxxxxxdoolcclcccllclc::::cllllllllllclooooodddx\n\
llllllllllllllllllllldKOkxxxdlloddxkkxxxxxdollccclllcllllc::::cclllllllllc:;cllllllll\n\
lllllllllllllllllllllloxkkkxdolloddxKX0xdool:c::clxKOollc:::::cccccccccccccccccccclll\n\
ooooooooooooooooooooooookkkkkddloddddxkdxdoc;;;;llooccccc:::ccccllllllllllcccllllllll\n\
ooooooooooooooooooooooooxkkkkkxdodddxxkxdoolcccclllc:ccc:cccclllllllccccccccccccccccc\n\
llllllllllllllllllllllllokkkkkkxddxxxxxxxxddoooolcc::c:::ccccccccccccccccccc::::ccccc\n\
ccccccccccccccccccccclllldkkkkkkxddxddddddddddolcc:::::::cccclcccccccccccccccccccllll\n\
cccccccccllclccclllllllllokkkkkkkkxddoooollcccc::::;;::ccccccllllllllllllllllllllllll\n\
lllllllllllllllllllllllllokkkkkkkkkkddollc:::::::::::cccc:::cllllllllllllllllllllllll\n\
llllllllllllllllllllllllllxkkkkkkkkkkkkxoolccclllllllcc:::::cllllllllllllllllllllllll\n\
lllllllllllllllllllllllllldkkkkkkOOOkkkkkxddddddoollcc:::::::ccllllllllllllllllllllll\n\
cccllllllllllllllllllllllldxxkkkkkkkkkkOkkxxdollccc:::;;;;;:::::cccccllllllllllllllll\n\
llllllllllcllcllllllllcccdxxxxxxxxxxxxxddolcc::::::::;;;;;;;;;;;;;::,.,;:lllllllloooo\n\
lllllllllllllllllllcc:;;cxxxxxxxddddddddoolc::::::;;;;;;;;;;;;;;;,,;.    .,cllloooooo\n\
llllllllllllllllc:;;,,,,;oddddddddoooddooolc::::;;;;;;;;;;;;;;;;,,,'        ..,:lllll\n\
lllllllccccc:;;,,,,,,,,,,;ldddddddoooooolllcc:::;;;;;;;;;;;;;;,,,,;.            ..,;c\n\
cccccc:;,'''.............'',cooodddolllllcccc:::;;;;;;;;;,,,,,,,;,.                  \n\
,,''.........................',:lddolc:cccc:::::;;;;;;,,,,,,,,;'.                    \n\
.................................,col:;;;;::::::;;,,,,,,,,,'..                       \n\
........................................'',,;;;;;,,,,''...                           \n\
.............................              ......                                    \n\
..........................                                                           \n\
      ....................                                                           \n\
       ..................                                                            \n\
          ... .   ..  ...                                                            \n")
            lwhText.pack()
            lwh.mainloop()
        else:
            message=sshcommand("rsh " + nodeVar.get() + " '"+command+"'")
            setText(message+'\n')
    t=threading.Thread(target=putcommand)
    t.start()

def classify():
    '''
    Identify
    '''
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
        sf = abs(np.fft.fftn(pha[0],[nx*d,ny*d,nz*d]))[0:int(4*d),0:int(4*d),0:int(4*d)]
        
        sf /= sf[0][0][0]
        return sf

    from sklearn.externals import joblib
    import numpy as np

    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    nx=int(result[18])
    ny=int(result[19])
    nz=int(result[20])

    # Load the saved learning result
    with open('classifier.pkl','rb') as file:
        classifier=joblib.load(file)

    with open('estimator.pkl','rb') as file:
        estimator=joblib.load(file)

    # Load a file and convert to fft matrix
    matrix = np.array(geteig('./pha.dat',nx,ny,nz)).reshape((1,-1))

    # Print the result
    # print(estimator.predict(matrix))
    if estimator.predict(matrix)[0] > 0:
        setText(classifier.predict(matrix)[0]+'\n')
        setText(str(round(max(classifier.predict_proba(matrix)[0]),3)*100)+'%\n')
    else:
        setText('not a known phase\n')
    
def vmstat():
    message=sshcommand("rsh " + nodeVar.get() + " 'vmstat'")
    setText(nodeVar.get() + ": " + message.strip('\n').split('\n')[-1].strip(' ').split(' ')[0]+' tasks running\n')

if __name__ == '__main__':
    #Initializing GUI
    root = Tk()
    root.resizable(width=False, height=False)
    root.title("ABCDEN")
    root.iconbitmap('icon.ico')
    
    labels = ['Initiation method','Anderson Mixing Option','Max Normal Mixing Steps','wopt','wcmp','maxErr','(ly/lx)^2','(lz/lx)^2','Matrix','hAB','hBC','hAC','fA','fB','fC','lx','ly','lz','Nx','Ny','Nz','Parameters Filename','Concentration Filename','ds0','epA','epB','epC','path']

    #Defining global variables to use in different functions
    defaults=[]
    result=[]
    eng=None

    #Read paras from file
    para = open('para','r')
    for item in para.readlines():
        for element in item.strip('\n').split(', '):
            defaults.append(element)
    para.close()

    entry=[]
    frame = Frame(root)

    for i in range(len(labels)):
        entry.append(Entry(frame))
        Label(frame, text=labels[i]).grid(row=i//3,column=(i%3)*2)
        entry[i].insert(10,defaults[i])
        entry[i].grid(row=i//3,column=(i%3)*2+1)

    buttonframe = Frame(root)
    Button(buttonframe, text='generate',command=generate,width=15).grid(row=0,column=0,padx=5,pady=5)
    Button(buttonframe, text='update',command=update,width=15).grid(row=0,column=1,padx=5,pady=5)
    Button(buttonframe, text='run',command=run,width=15).grid(row=0,column=2,padx=5,pady=5)
    Button(buttonframe, text='multirun',command=openmultirun,width=15).grid(row=0,column=3,padx=5,pady=5)
    Button(buttonframe, text='multilx',command=openmultilx,width=15).grid(row=0,column=4,padx=5,pady=5)
    Button(buttonframe, text='top',command=top,width=15).grid(row=1,column=0,padx=5,pady=5)
    Button(buttonframe, text='getpha',command=getfile,width=15).grid(row=1,column=1,padx=5,pady=5)
    Button(buttonframe, text='classify',command=classify,width=15).grid(row=1,column=2,padx=5,pady=5)
    Button(buttonframe, text='mayavi',command=mayavi,width=15).grid(row=1,column=4,padx=5,pady=5)
    Button(buttonframe, text='printout',command=printout,width=15).grid(row=0,column=5,padx=5,pady=5)
    Button(buttonframe, text='autoremove',command=autoremove,width=15).grid(row=1,column=3,padx=5,pady=5)
    btmatlab = Button(buttonframe, text='matlab',command=matlab,width=15)

    nodeframe = Frame(root)
    nodeVar=StringVar(root)
    nodeVar.set('c0102')
    nodes=['c0102','c0103','c0105','c0106','c0108','c0109','c0110','c0111','c0112','c0113']
    for i, item in enumerate(nodes):
        Radiobutton(nodeframe, text=item, variable=nodeVar, value=item,command=vmstat).grid(row=0,column=i,padx=5)

    resultText = Text(root,height=20,width=85)
    cmdEntry = Entry(root,width=85)
    cmdEntry.bind('<Key-Return>', entryCommand)
    setText("WELCOME TO ABCDEN\n")
    
    frame.pack()
    buttonframe.pack()
    nodeframe.pack()
    resultText.pack()
    cmdEntry.pack()

    #Load Matlab
    t=threading.Thread(target=loadmatlab)
    t.start()

    #Load sftp
    # private_key = paramiko.RSAKey.from_private_key_file('C:/Users/Birfy/.ssh/id_rsa')
    transport = paramiko.Transport(('10.158.131.62', 22))
    # transport.connect(username='NXW', pkey=private_key)
    transport.connect(username='cdy', password='birfy19970405')
    sftp = paramiko.SFTPClient.from_transport(transport)
    setText('SFTP IS LOADED\n')

    #Load ssh
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname='10.158.131.62',port=22,username='cdy',password='birfy19970405')
    # ssh.connect(hostname='10.158.131.62',port=22,username='NXW')
    setText('SSH IS LOADED\n')
    
    vmstat()

    root.mainloop()

    #Close the services
    ssh.close()
    transport.close()
    eng.quit()