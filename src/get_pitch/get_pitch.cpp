/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <algorithm>

#include "wavfile_mono.h"
#include "pitch_analyzer.h"
#include "digital_filter.h"

#include "docopt.h"
#include "ffft/FFTReal.h"
#include "ffft/FFTReal.hpp"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */
#define CLIPPING_THRESHOLD 0.007

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Detector 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the detection:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

  // Define analyzer
  PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::HAMMING, 50, 500);

  /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used.

  // Center-clipping con offset ======> Mejores valores con CLIPPING_THRESHOLD = 0.003  
  #if 0     
    for(unsigned int i = 0; i < x.size(); i++){
    if(x[i] > CLIPPING_THRESHOLD)         x[i] -= CLIPPING_THRESHOLD;
    else if(x[i] < - CLIPPING_THRESHOLD)  x[i] += CLIPPING_THRESHOLD;
    else                                  x[i] = 0; 
    }
  #endif

  // !!!!!!***** Usamos este porqué da mejor media *****!!!!!
  // Center-clipping sin offset ======> Mejores valores con CLIPPING_THRESHOLD = 0.007  
  #if 1
   for(unsigned int i = 0; i < x.size(); i++) {
    if(x[i] < CLIPPING_THRESHOLD && x[i] > - CLIPPING_THRESHOLD)  x[i] = 0;                                
   }
  #endif
  
  //high-pass filter
  int fc_low = 43;
  float a_low = 2*3.141592*fc_low/(2*rate);
  vector<float> a1 = {1, -1};
  vector<float> b1 = {1+a_low, -1+a_low};
  DigitalFilter filter (b1, a1, 1);
  x = filter(x);

  //low-pass filter
  int fc_high = 1925;
  float a_high = 2*3.141592*fc_high/(2*rate); 
  vector<float> a2 = {a_high, a_high};
  vector<float> b2 = {1+a_high, -1+a_high};

  DigitalFilter filter2 (b2, a2, 1);
  x = filter2(x);


  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  /// \TODO
  /// Postprocess the estimation in order to supress errors. For instance, a median filter
  /// or time-warping may be used.


    /*vector<int> our_vector{3, 5, 4, 1, 2};
    sort(our_vector.begin(), our_vector.end());
    for(unsigned int i = 0; i < our_vector.size(); i++)  cout << our_vector[i] << endl; 
    */

    for(unsigned int i = 1; i < f0.size()-1; i++){
        vector<float> our_vector{f0[i-1], f0[i], f0[i+1]};
        sort(our_vector.begin(), our_vector.end());
        f0[i] = our_vector[1];
    }

    /*for (int i = 1; i < (f0.size() - 1); i++){
    aux[0] = f0[i-1];
    aux[1] = f0[i];
    aux[2] = f0[i+1];
    sort(aux.begin(), aux.end());
    f0[i] = aux[1];
  }*/


  /* for (int i = 1; i<((int)f0.size()-1); i++){
    if (f0[i+1] != 0 && f0[i-1]!=0)   f0[i] = (f0[i-1]+f0[i+1])/2;
  }*/

  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}
