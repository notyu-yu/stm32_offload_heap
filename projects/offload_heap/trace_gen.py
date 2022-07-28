# python trace_gen.py l v file
# Create a random trace file with l lines, v variables, and write it to file

import sys
import random

lines = int(sys.argv[1]);
variables = int(sys.argv[2]);
file = sys.argv[3];

uninit_var = [i for i in range(variables)]
alloced_var = []

# Return random size between 1 and 300 bytes
def random_size():
    return random.randint(1, 300);

# Return random operation with arguments
def random_op():
    if not alloced_var:
        if uninit_var:
            key = 1
        else:
            return 's'
    elif not uninit_var:
        key = random.randint(2,3);
    else:
        key = random.randint(1,3);
    match key:
        case 1:
            var = uninit_var.pop(0)
            alloced_var.append(var)
            return f'a {var} {random_size()}'
        case 2:
            var = alloced_var.pop(0)
            return f'f {var}'
            return 'f'
        case 3:
            var = random.sample(alloced_var, 1)[0]
            return f'r {var} {random_size()}'

with open(file, 'w+') as f:
    file_str = ''
    written_lines = 0

    for _ in range(lines):
        if uninit_var or alloced_var:
            file_str += "\n" + random_op()
            written_lines += 1
        else:
            break
    
    file_str = f'20000\n{variables}\n{written_lines}\n1' + file_str
    f.write(file_str)
