/// @file

#include <iostream>
#include <math.h>
#include "pitch_analyzer.h"
#include "ffft/FFTReal.h"
#include "ffft/FFTReal.hpp"

using namespace std;

/// Name space of UPC
namespace upc {
  float noise_power = 0;
  float frame_number = 0;
  const int N = 4096;
  float f[N];
  ffft::FFTReal <float> fft_object (N);
  
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const { 
        //HECHO compute the autocorrelation r[l]
    for (unsigned int k = 0; k < r.size(); ++k) {
        for(unsigned int i = 0; i < (x.size() - k); ++i){ 
            r[k] += x[i] * x[i+k];
        }
      r[k] /= x.size(); 
    }
    //for (unsigned int k = 0; k < r.size(); ++k) cout << "Muestra de r" << k << ": " << r[k] << '\n';
    //for (unsigned int i = 0; i < x.size(); ++i) cout << "Muestra de x" << i << ": " << x[i] << '\n';


    if (r[0] == 0.0F) //to avoid log() and divide zero 
      r[0] = 1e-10; 
  }

  void PitchAnalyzer::amdf(const vector<float> &x, vector<float> &amdf) const {
    for (unsigned int k = 0; k < amdf.size(); ++k){
      for(unsigned int i = 0; i < x.size() - k ; ++i){  
          //amdf[k] += fabs(x[i] - x[i+k]);
          amdf[k] += (fabs(x[i] - x[i+k])) * (fabs(x[i] - x[i+k])) * (fabs(x[i] - x[i+k])) ;
        }
    }
    
    //for (unsigned int k = 0; k < amdf.size(); ++k) cout << "Muestra de amdf" << k << ": " << amdf[k] << '\n';
    //for (unsigned int i = 0; i < x.size(); ++i) cout << "Muestra de x" << i << ": " << x[i] << '\n';

  }

  void PitchAnalyzer::cepstrum(const vector<float> &x, vector<float> &r) const { 
    
      
      fft_object.do_fft (f, &x[0]);
      for(unsigned int i = 0; i < N; i++){
        cout << f[i] << endl;
        f[i] = log10(fabs(f[i]));
      }
      fft_object.do_ifft(f, &r[0]);


  }

  void PitchAnalyzer::set_window(Window win_type) {
    if (frameLen == 0)
      return;

    window.resize(frameLen);

    switch (win_type) {
      case HAMMING:
      /// \HECHO Implement the Hamming window
       // Finestra de Hamming
        for(int i = 0; i < window.size(); i++) {
            window[i] = 0.53836 - 0.46164 * cos(2 * 3.1415926535898 * i / (window.size()-1));  //float a0 = 0.53836; float a1 = 0.46164;
        }

      break;
      
      case RECT:
    
      default:
        window.assign(frameLen, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0) {
    npitch_min = (unsigned int) samplingFreq/max_F0;
    if (npitch_min < 2)
      npitch_min = 2;  // samplingFreq/2

    npitch_max = 1 + (unsigned int) samplingFreq/min_F0;

    //frameLen should include at least 2*T0
    if (npitch_max > frameLen/2)
      npitch_max = frameLen/2;
  }

  bool PitchAnalyzer::unvoiced(float pot, float r1norm, float rmaxnorm) const {
    /// \TODO Implement a rule to decide whether the sound is voiced or not.
    /// * You can use the standard features (pot, r1norm, rmaxnorm),
    ///   or compute and use other ones.

    // Case voiced  
      if ((r1norm > 0.866 && rmaxnorm > 0.294 && pot > noise_power))  return false;

    // Case unvoiced
    return true;
  }

  float PitchAnalyzer::compute_pitch(vector<float> & x) const {
    if (x.size() != frameLen)
      return -1.0F;

    //Window input frame
    for (unsigned int i=0; i<x.size(); ++i)
      x[i] *= window[i];

    vector<float> r(npitch_max);             //npitch_max = K
    //vector<float> c(npitch_max);             //npitch_max = K
    vector<float> amdf_var(npitch_max);      //npitch_max = K

    //Compute cepstrum
    //cepstrum(x, c);

    //Compute correlation
    autocorrelation(x, r);
        
    //Compute amdf
    amdf(x, amdf_var);

    if (frame_number == 0)  noise_power = 10*log10(r[0]) * 0.93;

    frame_number++;

    vector<float>::const_iterator iR = r.begin() + npitch_min, iRMax = iR;     // In either case, the lag should not exceed that of the minimum value of the pitch.
    /// \HECHO 
    /// Find the lag of the maximum value of the autocorrelation away from the origin.<br>

    while(iR != r.end()){     
      if(*iR > *iRMax)  iRMax = iR;       // Posición donde está el máximo 
      iR++;                               // Aumentamos iterador para ver si encontramos otro.
    }

    /// The lag corresponding to the maximum value of the pitch.
    unsigned int lag = iRMax - r.begin();
    


    vector<float>::const_iterator iR2 = amdf_var.begin() + npitch_min, iRMin = iR2;    

    
    for (unsigned int lag_amdf = npitch_min; lag_amdf < npitch_max; lag_amdf++){
      if(*iR2 < *iRMin)    iRMin = iR2;
      iR2++;
    }

    unsigned int lag_amdf = iRMin - amdf_var.begin();

    
    //cout << "mate: " << (float) samplingFreq / lag_amdf << endl;  



  #if 0
      cout << "valor autocorrelación: " << *iRMax << '\t' << "lag:" << lag << endl; 
  #endif

      float pot = 10 * log10(r[0]);

  #if 0
      if (r[0] > 0.0F)
        cout << "pow: " << pot << '\t' << "Th1: " <<r[1]/r[0] << '\t' << "Th2: " << r[lag]/r[0] << '\t' << " ZCR " << ZCR << endl;
  #endif 
    if (unvoiced(pot, r[1]/r[0], r[lag]/r[0])){
      return 0;
    }  
    else
      return (float) samplingFreq/(float) lag_amdf;
  }
}
