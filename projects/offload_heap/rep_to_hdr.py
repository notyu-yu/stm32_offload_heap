# Takes filename as argument, and convert trace file into teststring.h

import sys

file = sys.argv[1].strip();

with open(file, 'r') as f:
    f_str = f.read()
    hdr_str = "#define TESTSTRING \""
    #Replace newline with escaped character
    hdr_str += f_str.replace("\n", "\\n")
    hdr_str += '"'
    with open("mcu_side/teststring.h", "w+") as hf:
        hf.write(hdr_str)

print(f'teststring.h created from {file}')
