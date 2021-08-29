import matplotlib.pyplot as plt
import sys

file_name = sys.argv[1:][0]

print(file_name)

with open(file_name) as f:
    data = f.read()

data = data.split('\n')

x = [float(row) for row in data]

print(x)

plt.hist(x, density=True, bins=20)

output_file = file_name.split('.')[0] + ".png"

plt.savefig(output_file)