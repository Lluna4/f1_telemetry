import pandas as pd
import matplotlib.pyplot as plt
import numpy
from scipy.signal import savgol_filter 

a = pd.read_csv(r"C:\Users\carly\source\repos\f1_23_net\x64\Release\a.csv", sep=",")
print(a)

x1 = a[["WearRF"]].apply(savgol_filter,  window_length=631, polyorder=2)
x2 = a[["WearLF"]].apply(savgol_filter,  window_length=631, polyorder=2)
x3 = a[["WearRB"]].apply(savgol_filter,  window_length=631, polyorder=2)
x4 = a[["WearLB"]].apply(savgol_filter,  window_length=631, polyorder=2)

print(x1.head())
y = a.get("time")

plt.plot(y, x1, label  = "WearRF")
plt.plot(y, x2, label  = "WearLF")
plt.plot(y, x3, label  = "WearRB")
plt.plot(y, x4, label  = "WearLB")

plt.legend()
plt.show()
