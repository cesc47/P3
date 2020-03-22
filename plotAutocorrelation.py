import matplotlib.pyplot as plt 
import soundfile as sf
import numpy as np

senyal, fm = sf.read('a3.wav')                   # Read signal
t = np.arange(0, len(senyal)) / fm              # Create time axis

fig, (ax1, ax2) = plt.subplots(2, 1)            # Subplot: ax1-> Time signal ax2->Autocorrelation
fig.subplots_adjust(hspace=0.5)                 # Make a little extra space between the subplots


autocorrelation = t                             # Make a copy of the vector -> in there we will calculate r(t)

for i in range(len(senyal)):
    for k in range(len(senyal)-i):
         autocorrelation[i] = autocorrelation[i] + senyal[k] * senyal[k+i-1]
    autocorrelation[i] = autocorrelation[i] / len(senyal)

ax1.plot(senyal)
ax1.set_title('Señal temporal')
ax1.set_xlabel('Samples')
ax1.set_ylabel('Amplitude')
ax1.grid(True)

ax2.plot(autocorrelation)
ax2.set_title('Autocorrelación')
ax2.set_xlabel('Samples')
ax2.set_ylabel('r(k)')
ax2.grid(True)

plt.show()
