import sys
import os.path
import re
import json

SET_RE = re.compile(r'^set\(([A-Z_]+)\n([^\)]+)\)', re.MULTILINE)

os.chdir(os.path.join(os.path.dirname(__file__), '../third_party/cef/libcef_dll'))

vars = {}

with open('CMakeLists.txt', 'r') as stream:
    for m in SET_RE.finditer(stream.read()):
        items = []
        for line in m.group(2).splitlines():
            line = line.strip()
            if line != '':
                items.append(os.path.abspath(line))

        vars[m.group(1)] = items

sys.stdout.write(json.dumps(vars))
