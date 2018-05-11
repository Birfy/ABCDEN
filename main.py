from tkinter import *
from numpy import arange
import os

root = Tk()
root.resizable(width=False, height=False)
root.title("ABCDEN")

labels = ['Initiation method','Anderson Mixing Option','Max Normal Mixing Steps','wopt','wcmp','maxErr','(ly/lx)^2','(lz/lx)^2','Matrix','hAB','hBC','hAC','fA','fB','fC','lx','ly','lz','Nx','Ny','Nz','Parameters Filename','Concentration Filename','ds0','epA','epB','epC','path']
defaults=[]
result=[]

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

def generate(fA='',fB='',fC=''):
    global result
    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    if fA:
        result[12]=fA
        result[13]=fB
        result[14]=fC

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
    resultText.insert(END,"PARA GENERATED\n")

def update():
    p = os.popen("bash update.bashrc")
    resultText.insert(END,p.read())
    

def run(path='',fA='',fB='',fC=''):
    generate(fA,fB,fC)

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
    resultText.insert(END,p.read())

    runfile = open('run.bashrc','w',newline='\n')
    for i in lines:
        runfile.write(i)
    runfile.close()

def openmultirun():
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
        fa=entryfa.get().strip()
        fb=entryfb.get().strip()
        fc=entryfc.get().strip()

        if v.get()==0:
            fb=eval('list(arange('+fb+'))')
            for i in fb:
                run(path=result[-1]+'/a'+"%.3f" % float(fa)+'b'+"%.3f" % i,fA="%.3f" % float(fa),fB="%.3f" % i,fC="%.3f" % (1-float(fa)-i))
        elif v.get()==1:
            fa=eval('list(arange('+fa+'))')
            for i in fa:
                run(path=result[-1]+'/a'+"%.3f" % i+'b'+"%.3f" % float(fb),fA="%.3f" % i,fB="%.3f" % float(fb),fC="%.3f" % (1-i-float(fb)))
        elif v.get()==2:
            fa=eval('list(arange('+fa+'))')
            for i in fa:
                run(path=result[-1]+'/a'+"%.3f" % i+'b'+"%.3f" % (1-i-float(fc)),fA="%.3f" % i,fB="%.3f" % (1-i-float(fc)),fC="%.3f" % float(fc))

    mrframe.pack()
    Button(mrroot,text='run',command=multirun,width=10).pack()
    mrroot.mainloop()

Button(buttonframe, text='generate',command=generate,width=10).grid(row=0,column=0,padx=5)
Button(buttonframe, text='update',command=update,width=10).grid(row=0,column=1,padx=5)
Button(buttonframe, text='run',command=run,width=10).grid(row=0,column=2,padx=5)
Button(buttonframe, text='multirun',command=openmultirun,width=10).grid(row=0,column=3,padx=5)

frame.pack()
buttonframe.pack()
resultText.pack()

root.mainloop()