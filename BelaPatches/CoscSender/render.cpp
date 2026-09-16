#include <Bela.h>
#include <libraries/Biquad/Biquad.h>
#include <libraries/OscSender/OscSender.h>
#include <libraries/Scope/Scope.h>
#include <libraries/Serial/Serial.h>

#define NUM_CHANNELS 1
#define OSCSENDPORT 30181
#define OSCSENDIP                                                              \
  "192.168.178.26" // CHANGE THIS TO MATCH RECCEIVER IP OR END ON 255 TO
                   // BROADCAST IN NETWORK

OscSender oscSender;

Scope scope;

std::vector<Biquad> lp(NUM_CHANNELS);

int signalChannel = 0;

int gInputChannels[NUM_CHANNELS] = {signalChannel};

float gLPfreq = 400.0;  // Cut-off frequency for low-pass filter (Hz)
float gFilterQ = 0.707; // Quality factor for the biquad filters to provide a
                        // Butterworth response

int gNoteState = 0;

float gKeyValues[NUM_CHANNELS] = {0.0}; // Signal coming in on each

// Variables for counting samples

int gPiezoPeakTime = 1000;
int gPiezoDebounceTime = 2500;

int gLookingForPiezoPeakCounter[NUM_CHANNELS] = {0}; // Counters for each piezo

int gPiezoState[NUM_CHANNELS] = {0}; // State of piezo reading
// States:
// 0: Waiting for input
// 1: Strike detected, collecting sample buffer
// 2: Do something with number (play back sample, send number, print it, etc)
// 3: Debounce and return to 0
float gFinalPiezoAmplitude[NUM_CHANNELS] = {
    0.0}; // Stores highest value found over time
int gLookingForPiezoPeak_counter[NUM_CHANNELS] = {
    0}; // Counts the 5ms needed to find the peak.
int gPiezoDebounce_counter[3] = {
    0}; // Counts the time that we stop the piezos so we don't re-trigger.
float gHighestPiezoValueFound[3] = {0}; // The highest value we found over time.
int gPiezoVels[NUM_CHANNELS] = {0};

int gStrikeCount[NUM_CHANNELS] = {0};

float thresh = 0.2;
// float gPeakDetector[NUM_CHANNELS] = { thresh };
float gPeakDetector[NUM_CHANNELS] = {thresh};
float gRolloff = 0.001;

// Analog Read Stuff

const unsigned int gResistiveSensorPin = 7;
float gAudioFramesPerAnalogFrame;
unsigned int gAnalogSendCounter = 0;
const unsigned int gAnalogSendEvery = 10;

float resistiveSensorValue = 0.0;

// HELPER FUNCTIONS

std::pair<float, float>
readPiezo(BelaContext *context, unsigned int,
          unsigned int); // returns {rawSample, processedSample}

bool setup(BelaContext *context, void *userData) {

  // setting up osc sender
  oscSender.setup(OSCSENDPORT, OSCSENDIP);

  // setting up scope
  scope.setup(3, context->audioSampleRate);

  // Set up low pass filter
  BiquadCoeff::Settings s = {
      .fs = context->audioSampleRate,
      .q = gFilterQ,
      .peakGainDb = 0,
  };
  for (unsigned int b = 0; b < lp.size(); ++b) {
    s.type = BiquadCoeff::lowpass;
    s.cutoff = gLPfreq;
    lp[b].setup(s);
  }

  // setting up analog read

  // Check if analog channels are enabled
  if (context->analogFrames == 0 ||
      context->analogFrames > context->audioFrames) {
    rt_printf(
        "Error: this example needs analog enabled, with 4 or 8 channels\n");
    return false;
  }

  // Useful calculations
  gAudioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;

  rt_printf("Setup Done\n");

  return true;
}

void render(BelaContext *context, void *userData) {

  float scopeSample[3] = {0.0};
  for (unsigned int n = 0; n < context->audioFrames; n++) {
    for (unsigned int k = 0; k < NUM_CHANNELS; k++) { //
      auto samples = readPiezo(context, n, gInputChannels[k]);
      float thisSample = samples.second; // work with processed sample

      scopeSample[k] = samples.first;

      /*
                  gInputs[k][gWrittenFrames] = samples.first; //record raw
         sample gOutputs[k][gWrittenFrames] = samples.second; //record filtered
         sample
      */

      gKeyValues[k] = thisSample;

      // 		// **** UNCOMMENT THIS TO WRITE AUDIO: ****

      // gOutputs[k][gWrittenFrames] = thisSample;
      // ++gWrittenFrames;
      // if(gWrittenFrames >= gOutputs[k].size()) {
      // // if we have processed enough samples an we have filled the
      // pre-allocated buffer,
      // // stop the program
      // 	rt_printf("TIME'S UP");
      // 	Bela_requestStop();
      // 	return;
      // }
      if (gPiezoState[k] == 0) {
        // 			// Process signal, wait for strike
        if (thisSample > thresh) {
          // gPeakDetector[k] = thisSample;
          gKeyValues[k] = thisSample;
          rt_printf("Strike detected on channel %d with velocity %d! Strike "
                    "number %d!\n",
                    k, gPiezoVels[k], gStrikeCount[k]++);
          // gHighestPiezoValueFound[k] = 0.0; // Reset the highest value
          gPiezoState[k] = 1;
        }

      } else if (gPiezoState[k] == 1) {
        // Strike detected, look for peak over gPiezoPeakTime samples
        gLookingForPiezoPeak_counter[k]++;
        if (gLookingForPiezoPeak_counter[k] < gPiezoPeakTime) {
          if (gHighestPiezoValueFound[k] <= thisSample) {
            gHighestPiezoValueFound[k] = thisSample;
          }
        } else {
          rt_printf("Highest found: %f\n", gHighestPiezoValueFound[k]);

          oscSender.newMessage("/bela/piezo/peak")
              .add((int)k)
              .add(gHighestPiezoValueFound[k])
              .send();

          gLookingForPiezoPeak_counter[k] = 0; // reset counter
          // gPeakDetector[k] = gHighestPiezoValueFound[k];
          gPiezoState[k] = 2;
        }
      } else if (gPiezoState[k] == 2) { // 2: SEND MIDI
        rt_printf("Highest value found: %f", gHighestPiezoValueFound[k]);
        gPiezoState[k] = 3; // Iterate state
      } else if (gPiezoState[k] == 3) {
        gHighestPiezoValueFound[k] = 0.0;
        gPiezoDebounce_counter[k]++;
        if (gPiezoDebounce_counter[k] > gPiezoDebounceTime) {
          gPiezoDebounce_counter[k] = 0;
          gPiezoState[k] = 0;
        }
      } // END STATE IF STATEMENT
      // scope.log(gKeyValues[0]);
    } // END CHANNEL FOR LOOP
    //++gWrittenFrames;

    // reading from analog Pin

    if (n % (unsigned int)gAudioFramesPerAnalogFrame == 0) {

      unsigned int analogFrame = n / (unsigned int)gAudioFramesPerAnalogFrame;

      resistiveSensorValue =
          analogRead(context, analogFrame, gResistiveSensorPin);

      gAnalogSendCounter++;

      if (gAnalogSendCounter >= gAnalogSendEvery) {
        oscSender.newMessage("/bela/analog/resistive")
            .add(resistiveSensorValue)
            .send();
        gAnalogSendCounter = 0;
      }
    }

    scope.log(scopeSample[0], scopeSample[1], scopeSample[2]);
  } // END FOR LOOP

  for (unsigned int n = 0; n < context->audioFrames; n++) {

    // AUDIO OUTPUT: Input is passed to output

    // Read from audio input 0, write to output 0 (gChest = 0)
    float audioOut = audioRead(context, n, signalChannel);

    // Write to output
    for (int channel = 0; channel < 2; channel++) {
      audioWrite(context, n, channel, audioOut);
    }

  } // END FOR LOOP
}

void cleanup(BelaContext *context, void *userData) {}

std::pair<float, float> readPiezo(BelaContext *context, unsigned int frame,
                                  unsigned int inputChannel) {
  float processedSample;
  float sample = audioRead(
      context, frame,
      inputChannel); // frame = this sample, inputChannel = channel number
  processedSample = lp[inputChannel].process(sample);

  // Rectify:
  if (processedSample < 0.0) {
    processedSample *= -1.0;
  }
  // scope.log(sample, processedSample);

  return {sample, processedSample};
}
