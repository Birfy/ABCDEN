from tkinter import Tk
from tkinter import Label
from tkinter import Button
from tkinter import Entry
import os
root = Tk()
root.title("Para Generator")

labels = ['Initiation method','Anderson Mixing Option','Max Normal Mixing Steps','wopt','wcmp','maxErr','(ly/lx)^2','(lz/lx)^2','Matrix','hAB','hBC','hAC','fA','fB','fC','lx','ly','lz','Nx','Ny','Nz','Parameters Filename','Concentration Filename','ds0','epA','epB','epC']
defaults = ['0','1','100','0.050','0.100','0.001','1', '1','0','40.0', '40.0', '40.0','0.14', '0.26', '0.60','4.000', '4.000', '4.000','64', '64', '64','fet.dat','pha.dat','0.005','1', '1', '1']
entry=[]

for i in range(len(labels)):
    entry.append(Entry(root))
    Label(root, text=labels[i]).grid(row=i//3,column=(i%3)*2)
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
    root.destroy()

Button(root, text='generate', command = generate).grid(row=14,column=3)
root.mainloop()