import threading
import paramiko
import time

def mail(filename):
    import smtplib
    from email.mime.text import MIMEText
    from email.header import Header
    
    # 第三方 SMTP 服务
    mail_host="smtp.ym.163.com"  #设置服务器
    mail_user="admin@birfied.com"    #用户名
    mail_pass="birfy19970405"   #口令 
    
    sender = 'admin@birfied.com'
    receivers = ['zzcdy@126.com']  # 接收邮件，可设置为你的QQ邮箱或者其他邮箱
    
    message = MIMEText(filename + ' is finished', 'plain', 'utf-8')
    
    subject = filename + ' is finished'
    message['Subject'] = Header(subject, 'utf-8')
    
    try:
        smtpObj = smtplib.SMTP() 
        smtpObj.connect(mail_host, 25)    # 25 为 SMTP 端口号
        smtpObj.login(mail_user,mail_pass)  
        smtpObj.sendmail(sender, receivers, message.as_string())
        print("successfully sent")
    except smtplib.SMTPException:
        print("Error")


def tiktock():
    print(time.asctime( time.localtime(time.time())))
    stdin, stdout, stderr = ssh.exec_command("rsh c0103 'ps -A w|grep 'a0\.[0-9]*b0\.[0-9]*x[0-9]\.[0-9]*''")
    message = stdout.read().decode().split('\n')
    files=[i.split(' ')[-1] for i in message if i]
    for item in files:
        if not item in runningfiles:
            runningfiles.append(item)
            print(item,' added')
    for item in runningfiles:
        if not item in files:
            runningfiles.remove(item)
            mail(item)
            print(item,' removed')
    timer=threading.Timer(300,tiktock)
    timer.start()

if __name__ == '__main__':
    runningfiles=[]

    ssh=paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(hostname='10.158.131.62',port=22,username='cdy')

    tiktock()