import numpy as np
import matplotlib.pyplot as plt

sample_rate = 1e6

# Generate tone plus noise
t = np.arange(1024*1000)/sample_rate
f = 50e3 # freq of tone
x = np.sin(2*np.pi*f*t) + 0.2*np.random.randn(len(t))
plt.plot(t[:200] * 1e6, x[:200])
plt.xlabel("Time (s)")
plt.ylabel("Applitude")
plt.title("Sine signal + noise")
plt.show()