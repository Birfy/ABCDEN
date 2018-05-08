ssh cdy@10.158.131.62 "mkdir $path"
scp ./para cdy@10.158.131.62:$path
ssh cdy@10.158.131.62 "cp ./update/abcden $path;rsh c0103 'cd $path;nohup ./abcden >/dev/null 2>&1 &'"
echo "ABCDEN IS RUNNING"