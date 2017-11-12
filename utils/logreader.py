import matplotlib
import matplotlib.pyplot as plt

pth = "/home/lyra/console.log"
s = "ls"
data = list()

with open(pth, 'r') as f:
    lines = f.readlines()
    for l in lines:
        if l.startswith(s):
            if int(l.split(' ')[1]) < 400:
                data.append(int(l.split(' ')[1]))


plt.plot(data)
plt.show()
