import matplotlib.pyplot as plt 
import soundfile as sf
import numpy as np

senyal, fm = sf.read('hola.wav') 
t = np.arange(0, len(senyal)) / fm 
plt.plot(t, senyal)
plt.show()
plt.title("Hola")
file_length = len(senyal)
print(file_length)
