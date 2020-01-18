import sys
import subprocess

subprocess.check_call(["./protoc"] + sys.argv[1:])
