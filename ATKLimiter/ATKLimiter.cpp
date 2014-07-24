#include <cmath>

#include "ATKLimiter.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 1;

enum EParams
{
  kAttack = 0,
  kRelease,
  kThreshold,
  kSoftness,
  kMakeup,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kAttackX = 25,
  kAttackY = 31,
  kReleaseX = 94,
  kReleaseY = 31,
  kThresholdX = 163,
  kThresholdY = 31,
  kSoftnessX = 232,
  kSoftnessY = 31,
  kMakeupX = 301,
  kMakeupY = 31,
  kKnobFrames = 43
};

ATKLimiter::ATKLimiter(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
    inFilter(NULL, 1, 0, false), outFilter(NULL, 1, 0, false)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kAttack)->InitDouble("Attack", 10., 1., 100.0, 0.1, "ms");
  GetParam(kAttack)->SetShape(2.);
  GetParam(kRelease)->InitDouble("Release", 10, 1., 100.0, 0.1, "ms");
  GetParam(kRelease)->SetShape(2.);
  GetParam(kThreshold)->InitDouble("Threshold", 0., -40., 0.0, 0.1, "dB"); // threshold is actually power
  GetParam(kThreshold)->SetShape(2.);
  GetParam(kSoftness)->InitDouble("Softness", -2, -4, 0, 0.1, "-");
  GetParam(kSoftness)->SetShape(2.);
  GetParam(kMakeup)->InitDouble("Makeup Gain", 0, 0, 40, 0.1, "-"); // Makeup is expressed in amplitude
  GetParam(kMakeup)->SetShape(2.);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(COMPRESSOR_ID, COMPRESSOR_FN);

  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);

  pGraphics->AttachControl(new IKnobMultiControl(this, kAttackX, kAttackY, kAttack, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kReleaseX, kReleaseY, kRelease, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kThresholdX, kThresholdY, kThreshold, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kSoftnessX, kSoftnessY, kSoftness, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kMakeupX, kMakeupY, kMakeup, &knob));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  MakeDefaultPreset((char *) "-", kNumPrograms);
  
  powerFilter.set_input_port(0, &inFilter, 0);
  gainLimiterFilter.set_input_port(0, &powerFilter, 0);
  attackReleaseFilter.set_input_port(0, &gainLimiterFilter, 0);
  applyGainFilter.set_input_port(0, &attackReleaseFilter, 0);
  applyGainFilter.set_input_port(1, &inFilter, 0);
  volumeFilter.set_input_port(0, &applyGainFilter, 0);
  outFilter.set_input_port(0, &volumeFilter, 0);
  
  Reset();
}

ATKLimiter::~ATKLimiter() {}

void ATKLimiter::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  // Mutex is already locked for us.

  inFilter.set_pointer(inputs[0], nFrames);
  outFilter.set_pointer(outputs[0], nFrames);
  outFilter.process(nFrames);
}

void ATKLimiter::Reset()
{
  TRACE;
  IMutexLock lock(this);
  
  int sampling_rate = GetSampleRate();
  
  inFilter.set_input_sampling_rate(sampling_rate);
  inFilter.set_output_sampling_rate(sampling_rate);
  powerFilter.set_input_sampling_rate(sampling_rate);
  powerFilter.set_output_sampling_rate(sampling_rate);
  attackReleaseFilter.set_input_sampling_rate(sampling_rate);
  attackReleaseFilter.set_output_sampling_rate(sampling_rate);
  gainLimiterFilter.set_input_sampling_rate(sampling_rate);
  gainLimiterFilter.set_output_sampling_rate(sampling_rate);
  applyGainFilter.set_input_sampling_rate(sampling_rate);
  applyGainFilter.set_output_sampling_rate(sampling_rate);
  volumeFilter.set_input_sampling_rate(sampling_rate);
  volumeFilter.set_output_sampling_rate(sampling_rate);
  outFilter.set_input_sampling_rate(sampling_rate);
  outFilter.set_output_sampling_rate(sampling_rate);

  powerFilter.set_memory(0);
  attackReleaseFilter.set_release(std::exp(-1 / (GetParam(kAttack)->Value() * 1e-3 * GetSampleRate()))); // in ms
  attackReleaseFilter.set_attack(std::exp(-1 / (GetParam(kRelease)->Value() * 1e-3 * GetSampleRate()))); // in ms
}

void ATKLimiter::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

  switch (paramIdx)
  {
    case kThreshold:
      gainLimiterFilter.set_threshold(std::pow(10, GetParam(kThreshold)->Value() / 10));
      break;
    case kSoftness:
      gainLimiterFilter.set_softness(std::pow(10, GetParam(kSoftness)->Value()));
      break;
    case kAttack:
      attackReleaseFilter.set_release(std::exp(-1 / (GetParam(kAttack)->Value() * 1e-3 * GetSampleRate()))); // in ms
      break;
    case kRelease:
      attackReleaseFilter.set_attack(std::exp(-1 / (GetParam(kRelease)->Value() * 1e-3 * GetSampleRate()))); // in ms
      break;
    case kMakeup:
      volumeFilter.set_volume_db(GetParam(kMakeup)->Value());
      break;

    default:
      break;
  }
}
