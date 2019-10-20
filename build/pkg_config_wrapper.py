import sys
import subprocess
import json
import shlex

cmd = ['pkg-config'] + sys.argv[1:]
libs = shlex.split(subprocess.check_output(cmd + ['--libs']).decode('utf8', 'replace'))
cflags = shlex.split(subprocess.check_output(cmd + ['--cflags']).decode('utf8', 'replace'))

sys.stdout.write(json.dumps(locals('libs', 'cflags')))
