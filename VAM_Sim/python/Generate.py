import random

num_lines =  25
size = 100
with open('test.txt', 'w') as f:
    for s in range(num_lines):
         x = round(random.uniform(0, size), 1)
         y = round(random.uniform(0, size), 1)
         z = round(random.uniform(0, size), 1)

         f.write(f"{x},{y},{z}\n")

    