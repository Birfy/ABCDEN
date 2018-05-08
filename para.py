from tkinter import *
import os

root = Tk()
root.resizable(width=False, height=False)
root.title("Para Generator")

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

def generate():
    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    para = open('para','w')
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
    

def run():
    result=[]
    for i in range(len(labels)):
        result.append(entry[i].get().strip())

    runfile = open('run.bashrc','r')
    lines = runfile.readlines()

    runfile = open('run.bashrc','w')
    for i in lines:
        runfile.write(i.replace('$path',result[len(result)-1]))
    runfile.close()

    p = os.popen("bash run.bashrc")
    resultText.insert(END,p.read())

    runfile = open('run.bashrc','w')
    for i in lines:
        runfile.write(i)
    runfile.close()

Button(buttonframe, text='generate', command = generate).grid(row=0,column=0,padx=5)
Button(buttonframe, text='update', command = update).grid(row=0,column=1,padx=5)
Button(buttonframe, text='run', command = run).grid(row=0,column=2,padx=5)

frame.pack()
buttonframe.pack()
resultText.pack()

root.mainloop()