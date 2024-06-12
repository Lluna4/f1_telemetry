import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from scipy.signal import savgol_filter 

a = pd.read_csv(r"x64\Release\laps\laps_VERSTAPPEN", sep=",")
b = pd.read_csv(r"x64\Release\laps\laps_PIASTRI", sep=",")
print(a)


x1 = a[["LapTime"]]
x2 = b[["LapTime"]]

print(x1.head())
y = np.arange(4)
plt.plot(y, x1, label  = "Me")
plt.plot(y, x2, label  = "Verstappen")

plt.legend()
plt.show()
