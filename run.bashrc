ssh cdy@10.158.131.62 "mkdir $path;cp ./update/abcden $path;cp ./update/para $path;rsh c0103 'cd $path;nohup ./abcden >/dev/null 2>&1 &'"
echo "ABCDEN IS RUNNING"