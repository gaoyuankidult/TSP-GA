import pylab as p


data = [439.6,281.8,247.8,116,87.93,47.53,139.87,43.87]
x = [i+1 for i in range(len(data))]
p.plot(x,data)
p.xlabel("thread numbers")
p.ylabel("average iteration")
p.grid(True)
p.show()
