import sys
import os.path
import shutil

source = sys.argv[1]
dest = sys.argv[2]

if os.path.isdir(dest):
    shutil.rmtree(dest)
elif os.path.exists(dest):
    os.unlink(dest)

if os.path.isdir(source):
    shutil.copytree(source, dest)
else:
    shutil.copy2(source, dest)
