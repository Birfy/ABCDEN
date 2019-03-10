import smtplib
import os
import time
import threading

def mail(item):
    mail_host="mail.fudan.edu.cn"
    mail_user="15302010010"
    mail_pass="birfy19970405" 
        
    sender = '15302010010@fudan.edu.cn'
    receivers = ['zzcdy@126.com']
        
    try:
        smtpObj = smtplib.SMTP() 
        smtpObj.connect(mail_host, 25)
        smtpObj.login(mail_user,mail_pass)  
        smtpObj.sendmail(sender, receivers, 'Content-Type: text/plain; charset="utf-8"\nMIME-Version: 1.0\nContent-Transfer-Encoding: base64\nSubject: =?utf-8?q?'+item+' is finished!?=\naGVsbG8=')
    except smtplib.SMTPException:
        print('error')

def tiktock():
    file=open('log','a')
    file.write(time.asctime(time.localtime(time.time()))+'\n')
    message = os.popen("rsh c0103 'ps -A w|grep 'a0\.[0-9]*b0\.[0-9]*x[0-9]\.[0-9]*''").read().decode()
    files=[i.split(' ')[-1] for i in message.split('\n') if i]
    for item in files:
        if not item in runningfiles:
            runningfiles.append(item)
            file.write(item+' added\n')
    for item in runningfiles:
        if not item in files:
            runningfiles.remove(item)
            mail(item)
            file.write(item+' removed\n')
    file.close()
    timer=threading.Timer(300,tiktock)
    timer.start()

if __name__ == '__main__':
    runningfiles=[]
    tiktock()
