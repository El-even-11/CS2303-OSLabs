import matplotlib.pyplot as plt
import numpy as np
import math


labels = ['20', '40', '60', '80', '100']
normal = [22.259, 30.568, 35.777, 41.616, 50.145]
fifo = [7.441, 15.152, 21.938, 28.863, 37.169]
rr = [7.166, 15.089,21.776,28.040,37.493]
ras = [6.140,11.952,17.358,22.330,29.473]
maxy = 0

for i in range(len(normal)):
    normal[i] = 20*(i+1)/normal[i]
    maxy = max(maxy,normal[i])

for i in range(len(fifo)):
    fifo[i] = 20*(i+1)/fifo[i]
    maxy = max(maxy,fifo[i])

for i in range(len(rr)):
    rr[i] = 20*(i+1)/rr[i]
    maxy = max(maxy,rr[i])

for i in range(len(ras)):
    ras[i] = 20*(i+1)/ras[i]
    maxy = max(maxy,ras[i])

x = np.arange(len(labels))  # the label locations
width = 0.2  # the width of the bars

fig, ax = plt.subplots()
rects1 = ax.bar(x - 3*width/2-0.015, normal, width, label='NORMAL',color='#5DADE2')
rects2 = ax.bar(x - width/2-0.005, fifo, width, label='FIFO',color='#3498DB')
rects3 = ax.bar(x + width/2+0.005, rr, width, label='RR',color='#2874A6')
rects4 = ax.bar(x + 3*width/2+0.015, ras, width, label='RAS',color='#1B4F72')

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_xlabel('Task Number')
ax.set_ylabel('Throughput')
ax.set_title('Throughput Benchmark')
ax.set_xticks(x, labels)
ax.legend(bbox_to_anchor=(1.05, 0), loc=3, borderaxespad=0)

# ax.bar_label(rects1, padding=3)
# ax.bar_label(rects2, padding=3)

ax.set_ylim(0,math.ceil(maxy*1.1))

fig.tight_layout()

# plt.bar(range(len(normal)),normal, ec='w', ls='-', lw=0.01)
plt.show()