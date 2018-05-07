from tkinter import *
import os

root = Tk()
root.title("Para Generator")

labels = ['Initiation method','Anderson Mixing Option','Max Normal Mixing Steps','wopt','wcmp','maxErr','(ly/lx)^2','(lz/lx)^2','Matrix','hAB','hBC','hAC','fA','fB','fC','lx','ly','lz','Nx','Ny','Nz','Parameters Filename','Concentration Filename','ds0','epA','epB','epC']
defaults = ['0','1','100','0.050','0.100','0.001','1', '1','0','40.0', '40.0', '40.0','0.14', '0.26', '0.60','4.000', '4.000', '4.000','64', '64', '64','fet.dat','pha.dat','0.005','1', '1', '1']
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
    columns=[1,2,2,1,2,1,3,3,3,3,1,1,1,3]
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
    os.system("echo PARA GENERATED")
    resultText.insert(END,"PARA GENERATED\n")

def update():
    p = os.popen("bash update.bashrc")
    resultText.insert(END,p.read())
    

def run():
    resultText.insert(END,"RUNNING\n")
    p = os.popen("bash run.bashrc")
    resultText.insert(END,p.read())

Button(buttonframe, text='generate', command = generate).grid(row=0,column=0,padx=5)
Button(buttonframe, text='update', command = update).grid(row=0,column=1,padx=5)
Button(buttonframe, text='run', command = run).grid(row=0,column=2,padx=5)

frame.pack()
buttonframe.pack()
resultText.pack()

root.mainloop()