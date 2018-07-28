"""A basic (single function) API written using hug"""
import paramiko
from mayavi import mlab
import numpy as np
from numpy import arange
from sklearn.externals import joblib

from flask import Flask
from flask import request
from flask import render_template
from flask import url_for

mlab.options.offscreen = True

app = Flask(__name__)

@app.route('/', methods=['GET', 'POST'])
def home():
    return '''<h1>ABCDEN</h1><h3><a href="https://birfied.com">back to birfispace</h3>
            <p><a href="./printout">printout</a></p>
            <p><a href="./top">top</a></p>
            <p><a href="./mayavi">mayavi</a></p>  
            <p><a href="./multirun">multirun</a></p>  '''

@app.route('/printout', methods=['GET'])
def printout_form():
    return '''<p><a href="..">index</a></p>
              <form action="/printout" method="post">
              <p>path<input name="path"></p>
              <p><button type="submit">printout</button></p>
              </form>'''

@app.route('/printout', methods=['POST'])
def printout():
    return '''<p><a href="..">index</a></p>
              <form action="/printout" method="post">
              <p>path<input name="path"></p>
              <p><button type="submit">printout</button></p>
              </form>''' + '<p>'+sshcommand("rsh c0103 'cat "+request.form['path']+"/printout.txt'").replace('\n','</br>')+'</p>'

@app.route('/top', methods=['GET'])
def top_form():
    return '''<p><a href="..">index</a></p>
              <form action="/top" method="post">
              <p>
              <select name="node">
              <option value ="c0102">c0102</option>
              <option value ="c0103">c0103</option>
              <option value ="c0105">c0105</option>
              <option value ="c0106">c0106</option>
              <option value ="c0107">c0107</option>
              <option value ="c0108">c0108</option>
              <option value ="c0109">c0109</option>
              <option value ="c0110">c0110</option>
              <option value ="c0111">c0111</option>
              <option value ="c0112">c0112</option>
              <option value ="c0113">c0113</option>
              </select>
              </p>
              <p><button type="submit">top</button></p>
              </form>'''

@app.route('/top',  methods=['POST'])
def top():
    '''
    Get the running abden typed file 
    '''
    return '''<p><a href="..">index</a></p>
              <form action="/top" method="post">
              <p>
              <select name="node">
              <option value ="c0102">c0102</option>
              <option value ="c0103">c0103</option>
              <option value ="c0105">c0105</option>
              <option value ="c0106">c0106</option>
              <option value ="c0107">c0107</option>
              <option value ="c0108">c0108</option>
              <option value ="c0109">c0109</option>
              <option value ="c0110">c0110</option>
              <option value ="c0111">c0111</option>
              <option value ="c0112">c0112</option>
              <option value ="c0113">c0113</option>
              </select>
              </p>
              <p><button type="submit">top</button></p>
              </form>''' + '<p>' + request.form['node'] + ": " + sshcommand("rsh " + request.form['node'] + " 'vmstat'").strip('\n').split('\n')[-1].strip(' ').split(' ')[0]+' tasks running' +'</p>' + '<p>'+sshcommand("rsh " + request.form['node'] + " 'ps -A w|grep 'a0\.[0-9]*b0\.[0-9]*x[0-9]\.[0-9]*''").replace('\n','</br>')+'</p>'

@app.route('/mayavi', methods=['GET'])
def mayavi_form():
    return '''<p><a href="..">index</a></p>
              <form action="/mayavi" method="post">
              <p>path<input name="path"></p>
              <p>  nx<input name="nx">   ny<input name="ny">   nz<input name="nz"></p>
              <p><button type="submit">mayavi</button></p>
              </form>'''

@app.route('/mayavi',  methods=['POST'])
def mayavi():
    '''
    Get the running abden typed file 
    '''
    sftp.get(request.form['path']+"/pha.dat","./pha.dat")
    mayavi_plot(int(request.form['nx']),int(request.form['ny']),int(request.form['nz']),request.form['path'].split('/')[-2]+request.form['path'].split('/')[-1])

    return '''<p><a href="..">index</a></p>
              <form action="/mayavi" method="post">
              <p>path<input name="path"></p>
              <p>  nx<input name="nx">   ny<input name="ny">   nz<input name="nz"></p>
              <p><button type="submit">mayavi</button></p>
              </form>''' + "<img src='./static/" + request.form['path'].split('/')[-2] + request.form['path'].split('/')[-1] + ".png'>" + '<p>' + classify(int(request.form['nx']),int(request.form['ny']),int(request.form['nz'])) + '</p>'

@app.route('/multirun', methods=['GET'])
def multirun_form():
    return '''<p><a href="..">index</a></p>
              <form action="/multirun" method="post">
              <p>init<input name="init"> lylx<input name="lylx"> lzlx<input name="lzlx"></p>
              <p>  fa<input name="fa">   fb<input name="fb">   fc<input name="fc"></p>
              <p>fix
              <select name="fix">
              <option value ="fa">fa</option>
              <option value ="fb">fb</option>
              <option value ="fc">fc</option>
              </select>
              </p>
              <p>  lx<input name="lx">   ly<input name="ly">   lz<input name="lz"></p>
              <p>  nx<input name="nx">   ny<input name="ny">   nz<input name="nz"></p>
              <p>path<input name="path"></p>
              <p>node
              <select name="node">
              <option value ="c0102">c0102</option>
              <option value ="c0103">c0103</option>
              <option value ="c0105">c0105</option>
              <option value ="c0106">c0106</option>
              <option value ="c0107">c0107</option>
              <option value ="c0108">c0108</option>
              <option value ="c0109">c0109</option>
              <option value ="c0110">c0110</option>
              <option value ="c0111">c0111</option>
              <option value ="c0112">c0112</option>
              <option value ="c0113">c0113</option>
              </select>
              </p>
              <p><button type="submit">multirun</button></p>
              </form>'''

@app.route('/multirun', methods=['POST'])
def multirun():
    def calculatelx(fa, fb, fc):
        if request.form['init'] == '233':
            return (fa+fb/2+fc/4)/0.42
        elif request.form['init'] == '633':
            return (fa+fb/2+fc/4)/0.42
        elif request.form['init'] == '15':
            return (fa+fb/2+fc/4)/0.42
        elif request.form['init'] == '1619':
            return (fa+fb/2+fc/4)/0.36
        elif request.form['init'] == '7':
            return (fa+fb/2+fc/4)/0.515

    def run(path='',fA='',fB='',fC='',lx=0.0):
        '''
        Copy the abcden file from ./update to path and para file to path in Server
        Run the abcden file on path
        '''

        filepath=path

        generate(fA,fB,fC,lx)

        filename='/'+filepath.split('/')[-2]+filepath.split('/')[-1]
        
        sshcommand("mkdir " + filepath)
        sftp.put('./para', filepath+"/para")
        sshcommand("cp ./update/abcden " + filepath + filename + ";rsh " + request.form['node'] + " 'cd " + filepath + ";nohup ."+filename+" >/dev/null 2>&1 &'")
    
    def generate(fA='',fB='',fC='',lx=0.0):
        '''
        Read paras from Entries and write to para file
        If fA, fB, fC or lx is substituted if not empty
        '''
        result=['233', '1', '100', '0.050', '0.100', '0.001', '1', '1', '0', '40.0', '40.0', '40.0', '0.400', '0.020', '0.580', '5.074', '5.074', '5.074', '32', '32', '32', 'fet.dat', 'pha.dat', '0.005', '1', '1', '1', './nf']
        
        result[0]=request.form['init']
        result[6]=request.form['lylx']
        result[7]=request.form['lzlx']

        result[12]=fA
        result[13]=fB
        result[14]=fC
        
        result[15]="%.3f" % (lx*float(request.form['lx']))
        result[16]="%.3f" % (lx*float(request.form['ly']))
        result[17]="%.3f" % (lx*float(request.form['lz']))

        result[18]=request.form['nx']
        result[19]=request.form['ny']
        result[20]=request.form['nz']

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
    
    if request.form['fix']=='fa':
        fb=eval('list(arange('+request.form['fb']+'))')
        for i in fb:
            lx=calculatelx(float(request.form['fa']),i,1-float(request.form['fa'])-i)
            run(path=request.form['path']+'/'+request.form['init']+'/a'+"%.3f" % float(request.form['fa'])+'b'+"%.3f" % i+'x'+request.form['lx'],fA="%.3f" % float(request.form['fa']),fB="%.3f" % i,fC="%.3f" % (1-float(request.form['fa'])-i), lx=lx)
    elif request.form['fix']=='fb':
        fa=eval('list(arange('+request.form['fa']+'))')
        for i in fa:
            lx=calculatelx(i,float(request.form['fb']),1-float(request.form['fb'])-i)
            run(path=request.form['path']+'/'+request.form['init']+'/a'+"%.3f" % i+'b'+"%.3f" % float(request.form['fb'])+'x'+request.form['lx'],fA="%.3f" % i,fB="%.3f" % float(request.form['fb']),fC="%.3f" % (1-i-float(request.form['fb'])), lx=lx)
    elif request.form['fix']=='fc':
        fa=eval('list(arange('+request.form['fa']+'))')
        for i in fa:
            lx=calculatelx(i,1-i-float(request.form['fc']),float(request.form['fc']))
            run(path=request.form['path']+'/'+request.form['init']+'/a'+"%.3f" % i+'b'+"%.3f" % (1-i-float(request.form['fc']))+'x'+request.form['lx'],fA="%.3f" % i,fB="%.3f" % (1-i-float(request.form['fc'])),fC="%.3f" % float(request.form['fc']), lx=lx)

    return '''<p><a href="..">index</a></p>
              <form action="/multirun" method="post">
              <p>init<input name="init"> lylx<input name="lylx"> lzlx<input name="lzlx"></p>
              <p>  fa<input name="fa">   fb<input name="fb">   fc<input name="fc"></p>
              <p>fix
              <select name="fix">
              <option value ="fa">fa</option>
              <option value ="fb">fb</option>
              <option value ="fc">fc</option>
              </select>
              </p>
              <p>  lx<input name="lx">   ly<input name="ly">   lz<input name="lz"></p>
              <p>  nx<input name="nx">   ny<input name="ny">   nz<input name="nz"></p>
              <p>path<input name="path"></p>
              <p>node
              <select name="node">
              <option value ="c0102">c0102</option>
              <option value ="c0103">c0103</option>
              <option value ="c0105">c0105</option>
              <option value ="c0106">c0106</option>
              <option value ="c0107">c0107</option>
              <option value ="c0108">c0108</option>
              <option value ="c0109">c0109</option>
              <option value ="c0110">c0110</option>
              <option value ="c0111">c0111</option>
              <option value ="c0112">c0112</option>
              <option value ="c0113">c0113</option>
              </select>
              </p>
              <p><button type="submit">multirun</button></p>
              </form>''' + '<p>Running</p>'

def sshcommand(cmd):
        '''
        Proceed a ssh command and get result
        '''
        stdin, stdout, stderr = ssh.exec_command(cmd)
        message = stdout.read()
        if not message:
            message = stderr.read()
        return message.decode()

def mayavi_plot(nx,ny,nz,path):
    '''
    Using Mayavi to plot
    '''

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

    a=np.array(a).reshape(int(nx),int(ny),int(nz))
    b=np.array(b).reshape(int(nx),int(ny),int(nz))
    c=np.array(c).reshape(int(nx),int(ny),int(nz))

    amatrix=np.zeros((int(nx)*n,int(ny)*n,int(nz)*n))
    bmatrix=np.zeros((int(nx)*n,int(ny)*n,int(nz)*n))
    cmatrix=np.zeros((int(nx)*n,int(ny)*n,int(nz)*n))
    for i in range(int(nx)*n):
        for j in range(int(ny)*n):
            for k in range(int(nz)*n):
                amatrix[i][j][k]=a[i%int(nx)][j%int(ny)][k%int(nz)]
                bmatrix[i][j][k]=b[i%int(nx)][j%int(ny)][k%int(nz)]
                cmatrix[i][j][k]=c[i%int(nx)][j%int(ny)][k%int(nz)]

    src = mlab.pipeline.scalar_field(cmatrix)
    mlab.pipeline.iso_surface(src, opacity=1,contours=[0.5],color=(1,0,0))

    src = mlab.pipeline.scalar_field(bmatrix)
    mlab.pipeline.iso_surface(src, opacity=1,contours=[0.5],color=(0,1,0))

    src = mlab.pipeline.scalar_field(amatrix)
    mlab.pipeline.iso_surface(src, opacity=1,contours=[0.5],color=(0,0,1))

    mlab.savefig('./static/'+path+'.png')
    mlab.clf()
    mlab.close()

def classify(nx,ny,nz):
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
        sf = abs(np.fft.fftn(pha[0],[64*d,64*d,64*d]))[0:int(4*d),0:int(4*d),0:int(4*d)]
        
        sf /= sf[0][0][0]
        return sf

    # Load the saved learning result
    with open('classifier.pkl','rb') as file:
        classifier=joblib.load(file)

    # with open('estimator.pkl','rb') as file:
        # estimator=joblib.load(file)

    # Load a file and convert to fft matrix
    matrix = np.array(geteig('./pha.dat',nx,ny,nz)).reshape((1,-1))

    # Print the result
    # print(estimator.predict(matrix))
    # if estimator.predict(matrix)[0] > 0:
    return (classifier.predict(matrix)[0] + '\n' + str(round(max(classifier.predict_proba(matrix)[0]),3)*100)+'%')
    # else:
        # return('not a known phase')

if __name__ == '__main__':

    #Load sftp
    # private_key = paramiko.RSAKey.from_private_key_file('C:/Users/Birfy/.ssh/id_rsa')
    transport = paramiko.Transport(('10.158.131.62', 22))
    # transport.connect(username='NXW', pkey=private_key)
    transport.connect(username='cdy', password='birfy19970405')
    sftp = paramiko.SFTPClient.from_transport(transport)


    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname='10.158.131.62',port=22,username='cdy',password='birfy19970405')
    
    app.run()