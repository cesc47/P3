import matplotlib.pyplot as plt
import soundfile as sf 
import numpy as np 

senyal, fm = sf.read('sb044.wav')
detector = open("detector.f0", "r")
wavesurfer = open("wavesurfer.f0", "r")

fig, (ax1,ax2,ax3) = plt.subplots(3,1)
fig.subplots_adjust(hspace=0.5)

muestras_detector = []
muestras_wavesurfer = []

for line in detector:
    line = line.strip('\n')
    muestras_detector.append(int(float(line)))
detector.close

for line in wavesurfer:
    line = line.strip('\n')
    muestras_wavesurfer.append(int(float(line)))
wavesurfer.close

for i in range(len(muestras_detector)):
    print(muestras_detector[i])
ax1.plot(senyal)
ax1.set_title('Señal')
ax1.set_xlabel('Samples')
ax1.set_ylabel('Amplitude')
ax1.grid(True)

ax2.plot(muestras_detector)
ax2.set_title('Pitch obtenido con nuestro detector')
ax2.set_xlabel('Samples')
ax2.set_ylabel('Frequency (Hz)')
ax2.grid(True)

ax3.plot(muestras_wavesurfer)
ax3.set_title('Pitch obtenido con wavesurfer')
ax3.set_xlabel('Samples')
ax3.set_ylabel('Frequency (Hz)')
ax3.grid(True)

plt.show()
