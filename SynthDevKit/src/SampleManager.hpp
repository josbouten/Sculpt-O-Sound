#pragma once

#include <cstdint>
#include <string>
#include <vector>

#define MAX_SAMPLE_NAME 64

namespace SynthDevKit {

struct Sample {
  Sample(uint64_t _length, std::string _name, float *_values, float _pitch,
         uint32_t _sampleRate = 44100, bool _loop = false)
      : length(_length), name(_name), values(_values), pitch(_pitch),
        sampleRate(_sampleRate), loop(_loop) {}
  uint64_t length;
  std::string name;
  float *values;
  float pitch;
  uint32_t sampleRate;
  bool loop;
};

struct SampleManagerContext {
  SampleManagerContext( ) {
    currentPosition = -1;
    sample          = nullptr;
  };
  void reset( ) { currentPosition = 0; }
  bool ready( ) { return (sample && currentPosition + 1 < sample->length); }
  double currentPosition;
  Sample *sample;
};

class SampleManager {
public:
  static SampleManager *getInstance( );
  SampleManager(const SampleManager &) = delete;
  SampleManager &operator=(const SampleManager &) = delete;

  float step(SampleManagerContext *, float, uint32_t = 44100);
  void addSample(Sample *);
  Sample *selectSample(std::string);

private:
  std::vector<Sample *> samples;
  // used to remove the ability to new
  SampleManager( );
};

}; // namespace SynthDevKit
