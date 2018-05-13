'''
ABCDEN SCFT GUI PROGRAM
Package needed:
    Matlab API for Python
    numpy
'''

from tkinter import *
from numpy import arange
import os
import threading

root = Tk()
root.resizable(width=False, height=False)
root.title("ABCDEN")

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
buttonframe = Frame(root)
resultText = Text(root,height=20)

for i in range(len(labels)):
    entry.append(Entry(frame))
    Label(frame, text=labels[i]).grid(row=i//3,column=(i%3)*2)
    entry[i].insert(10,defaults[i])
    entry[i].grid(row=i//3,column=(i%3)*2+1)

def setText(text):
    resultText.insert(END,text)
    resultText.see(END)

def loadmatlab():
    '''
    Begin to load Matlab
    Matlab button added when finish loading
    '''
    global eng
    import matlab.engine
    setText('LOADINGMATLAB\n')
    eng=matlab.engine.start_matlab()
    btmatlab.grid(row=0,column=7,padx=5) #Button is added when finish loading matlab
    setText('MATLABLOADED\n')

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

def update():
    '''
    Upload para file to ./update in Linux
    '''
    def updateonline():
        setText('UPDATING\n')
        p = os.popen("bash update.bashrc")
        setText(p.read())
    
    t=threading.Thread(target=updateonline)
    t.start()
    

def run(path='',fA='',fB='',fC='',lx=0.0):
    '''
    Copy the abcden file from ./update to path and para file to path in Server
    Run the abcden file on path
    '''
    def runonline():
        setText('RUNNINGONLINE\n')

        generate(fA,fB,fC,lx)

        runfile = open('run.bashrc','r',newline='\n')
        lines = runfile.readlines()

        runfile = open('run.bashrc','w',newline='\n')
        for i in lines:
            if not path:
                runfile.write(i.replace('$path',result[len(result)-1]))
            else:
                runfile.write(i.replace('$path',path))
        runfile.close()

        p = os.popen("bash run.bashrc")
        setText(p.read())

        runfile = open('run.bashrc','w',newline='\n')
        for i in lines:
            runfile.write(i)
        runfile.close()

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

            runfile = open('getfree.bashrc','r',newline='\n')
            lines = runfile.readlines()

            runfile = open('getfree.bashrc','w',newline='\n')
            for j in lines:
                runfile.write(j.replace('$path',i))
            runfile.close()

            p = os.popen("bash getfree.bashrc")
            writefile.write(p.read())

            runfile = open('getfree.bashrc','w',newline='\n')
            for j in lines:
                runfile.write(j)
            runfile.close()
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

        runfile = open('getpha.bashrc','r',newline='\n')
        lines = runfile.readlines()

        runfile = open('getpha.bashrc','w',newline='\n')
        for i in lines:
            runfile.write(i.replace('$path',result[len(result)-1]))
        runfile.close()

        p = os.popen("bash getpha.bashrc")
        setText(p.read())
        runfile = open('getpha.bashrc','w',newline='\n')
        for i in lines:
            runfile.write(i)
        runfile.close()

    t=threading.Thread(target=download)
    t.start()

def matlab():
    '''
    Using matlab to plot
    '''
    def plot():
        setText('PLOTTING\n')

        result=[]
        for i in range(len(labels)):
            result.append(entry[i].get().strip())

        runfile = open('plotphabc.m','r',newline='\n')
        lines = runfile.readlines()

        runfile = open('plotphabc.m','w',newline='\n')
        for i in lines:
            runfile.write(i.replace('$Nx',result[18]).replace('$Ny',result[19]).replace('$Nz',result[20]))
        runfile.close()

        eng.plotphabc(nargout=0)

        runfile = open('plotphabc.m','w',newline='\n')
        for i in lines:
            runfile.write(i)
        runfile.close()        
    
    t=threading.Thread(target=plot)
    t.start()

def top():
    def gettop():
        setText('TOPPING\n')
        p=os.popen('bash top.bashrc')
        setText(p.read())
    t=threading.Thread(target=gettop)
    t.start()

Button(buttonframe, text='generate',command=generate,width=10).grid(row=0,column=0,padx=5)
Button(buttonframe, text='update',command=update,width=10).grid(row=0,column=1,padx=5)
Button(buttonframe, text='run',command=run,width=10).grid(row=0,column=2,padx=5)
Button(buttonframe, text='multirun',command=openmultirun,width=10).grid(row=0,column=3,padx=5)
Button(buttonframe, text='multilx',command=openmultilx,width=10).grid(row=0,column=4,padx=5)
Button(buttonframe, text='top',command=top,width=10).grid(row=0,column=5,padx=5)
Button(buttonframe, text='getpha',command=getfile,width=10).grid(row=0,column=6,padx=5)
btmatlab = Button(buttonframe, text='matlab',command=matlab,width=10)

frame.pack()
buttonframe.pack()
resultText.pack()

t=threading.Thread(target=loadmatlab)
t.start()

root.mainloop()
eng.quit()