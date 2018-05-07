ssh cdy@10.158.131.62 "cp ./update/abcden ./; cp ./update/para ./; rsh c0103 'nohup ./abcden >/dev/null 2>&1 &'"
echo 'ABCDEN IS RUNNING'