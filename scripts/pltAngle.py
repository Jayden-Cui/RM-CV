from matplotlib import pyplot as plt

with open('../model/angle.dat', 'r') as fp:
    data = fp.read()

lines = data.split('\n')

datas = []
for line in lines:
    n = float(line)
    datas.append(n)

plt.plot(datas)
plt.show()