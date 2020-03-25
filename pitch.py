import matplotlib.pyplot as plt
import soundfile as sf 
import numpy as np
import math
import wave
import os
import shutil
import pitch

VENTANA = 0.015 #duracion de cada fragmento para calcular el pitch

pitch_calculado = []
pitch_automatico = []

txt = open("sb044.f0", "r")
for line in txt:
   pitch_calculado.append(line)
txt.close
print("longitud f0 = {}".format(len(pitch_calculado)))

audio = wave.open("sb044.wav", "r")

frameRate = audio.getframerate()
nChannels = audio.getnchannels()
sampWidth = audio.getsampwidth()
nSamples = audio.getnframes()
compType = audio.getcomptype()
compName = audio.getcompname()
duration = nSamples/frameRate
samplesPorTrozo = round(nSamples/(duration/VENTANA))
numeroDeTrozos = math.floor((duration/VENTANA))
samplesSobrantes = nSamples - (numeroDeTrozos*samplesPorTrozo)

# Create a folder for the chunked .wav files
# If it exists we remove it and create a brand new one
if os.path.exists('WAV_chunked'):
   shutil.rmtree('WAV_chunked')
   os.mkdir('WAV_chunked')
else:
   os.mkdir('WAV_chunked')

posicion = 0
os.chdir('WAV_chunked')

for i in range(numeroDeTrozos):
   audio.setpos(posicion)
   frames = audio.readframes(samplesPorTrozo)
   fileName = "fragment" + str(i) + ".wav"
   with wave.open(fileName, "w") as chunkAudio:
      params = (nChannels, sampWidth, frameRate, samplesPorTrozo ,compType, compName)
      chunkAudio.setparams(params)
      chunkAudio.writeframes(frames)
      posicion += samplesPorTrozo


if samplesSobrantes  != 0:
   fileName = "fragment" + str(numeroDeTrozos) + ".wav"
   with wave.open(fileName, "w") as chunkAudio:
      params = (nChannels, sampWidth, frameRate, samplesSobrantes ,compType, compName)
      chunkAudio.setparams(params)
      chunkAudio.writeframes(frames)

for i in range(numeroDeTrozos):
   fileName = "fragment" + str(i) + ".wav"
   pitch_automatico.append(pitch.find_pitch(fileName))

print('----- FICHERO ORIGINAL -----')
print('numero de frames = {}'.format(nSamples))
print('samples = {}'.format(sampWidth))
print('duration = {}'.format(duration))
print('numero de trozos = {}'.format(numeroDeTrozos))
print('samples por trozo = {}'.format(samplesPorTrozo))
print('----- PRIMER FICHERO CORTADO -----')
print('samples = {}'.format(chunkAudio.getnframes()))

for i in range(numeroDeTrozos):
   print('nuestro ={}'.format(pitch_calculado[i]))
   print('\t')
   print('libreria ={}'.format(pitch_automatico[i]))






#f = sf.SoundFile('sb044.wav')
#print('longitud f0 = {}'.format(len(pitch_calculado)))
#print('samples = {}'.format(len(f)))
#print('sample rate = {}'.format(f.samplerate))
#print('seconds = {}'.format(len(f)/f.samplerate))
#print('numero de fracmentos del .wav = {}'.format(math.ceil((len(f)/f.samplerate)/0.015)))