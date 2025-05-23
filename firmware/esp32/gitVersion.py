from datetime import datetime
import subprocess
import sys

try:
    revision = (
        subprocess.check_output(["git", "describe", "--always", "--dirty"])
        .strip()
        .decode("utf-8")
    )
except Exception as e:
    revision = "no_git"
    
del sys.argv[0]
for arg in sys.argv:
    print(arg, end = ' ')

# datetime object containing current date and time
now = datetime.now()
formatted_datetime = now.strftime("%Y-%m-%d %H:%M:%S")
print("-DGIT_REV='\"%s %s\"'" % (revision, formatted_datetime) )