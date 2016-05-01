#include <cmath>

#include "ATKTransientShaper.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "controls.h"
#include "resource.h"

const int kNumPrograms = 2;

enum EParams
{
  kPower = 0,
  kAttack,
  kAttackRatio,
  kRelease,
  kReleaseRatio,
  kThreshold,
  kSlope,
  kSoftness,
  kColored,
  kQuality,
  kMakeup,
  kDryWet,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kPowerX = 27,
  kPowerY = 40,
  kAttackX = 130,
  kAttackY = 40,
  kReleaseX = 233,
  kReleaseY = 40,
  kThresholdX = 336,
  kThresholdY = 40,
  kSlopeX = 439,
  kSlopeY = 40,
  kSoftnessX = 542,
  kSoftnessY = 40,
  kColoredX = 645,
  kColoredY = 40,
  kQualityX = 748,
  kQualityY = 40,
  kMakeupX = 851,
  kMakeupY = 40,
  kDryWetX = 954,
  kDryWetY = 40,
  
  kKnobFrames = 20,
  kKnobFrames1 = 19
};

ATKTransientShaper::ATKTransientShaper(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
inFilter(nullptr, 1, 0, false), outFilter(nullptr, 1, 0, false), gainCompressorFilter(1, 256*1024)
{
  TRACE;
  
  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kPower)->InitDouble("Power", 10., 0., 100.0, 0.1, "ms");
  GetParam(kPower)->SetShape(2.);
  GetParam(kAttack)->InitDouble("Attack", 10., 1., 100.0, 0.1, "ms");
  GetParam(kAttack)->SetShape(2.);
  GetParam(kAttackRatio)->InitDouble("Attack Ratio", 10., 0., 100.0, 0.1, "%");
  GetParam(kRelease)->InitDouble("Release", 10, 1., 100.0, 0.1, "ms");
  GetParam(kRelease)->SetShape(2.);
  GetParam(kReleaseRatio)->InitDouble("Release Ratio", 10, 0., 100.0, 0.1, "%");
  GetParam(kThreshold)->InitDouble("Threshold", 0., -40., 0.0, 0.1, "dB"); // threshold is actually power
  GetParam(kSlope)->InitDouble("Slope", 2., 1.5, 100, .1, "-");
  GetParam(kSlope)->SetShape(2.);
  GetParam(kColored)->InitDouble("Color", 0, -.5, .5, 0.01, "-");
  GetParam(kQuality)->InitDouble("Quality", 0.1, 0.01, .2, 0.01, "-");
  GetParam(kSoftness)->InitDouble("Softness", -2, -4, 0, 0.1, "-");
  GetParam(kSoftness)->SetShape(2.);
  GetParam(kMakeup)->InitDouble("Makeup Gain", 0, 0, 40, 0.1, "dB"); // Makeup is expressed in amplitude
  GetParam(kDryWet)->InitDouble("Dry/Wet", 1, 0, 1, 0.01, "-");
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(COLORED_COMPRESSOR_ID, COLORED_COMPRESSOR_FN);
  
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  IBitmap knob1 = pGraphics->LoadIBitmap(KNOB1_ID, KNOB1_FN, kKnobFrames1);
  IColor color = IColor(255, 255, 255, 255);
  IText text = IText(10, &color, nullptr, IText::kStyleBold);
  
  pGraphics->AttachControl(new IKnobMultiControlText(this, IRECT(kPowerX, kPowerY, kPowerX + 78, kPowerY + 78 + 21), kPower, &knob, &text, "ms"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, IRECT(kAttackX, kAttackY, kAttackX + 78, kAttackY + 78 + 21), kAttack, &knob, &text, "ms"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, IRECT(kReleaseX, kReleaseY, kReleaseX + 78, kReleaseY + 78 + 21), kRelease, &knob, &text, "ms"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, IRECT(kThresholdX, kThresholdY, kThresholdX + 78, kThresholdY + 78 + 21), kThreshold, &knob, &text, "dB"));
  pGraphics->AttachControl(new IKnobMultiControlText(this, IRECT(kSlopeX, kSlopeY, kSlopeX + 78, kSlopeY + 78 + 21), kSlope, &knob, &text, ""));
  pGraphics->AttachControl(new IKnobMultiControl(this, kSoftnessX, kSoftnessY, kSoftness, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, kColoredX, kColoredY, kColored, &knob1));
  pGraphics->AttachControl(new IKnobMultiControl(this, kQualityX, kQualityY, kQuality, &knob));
  pGraphics->AttachControl(new IKnobMultiControlText(this, IRECT(kMakeupX, kMakeupY, kMakeupX + 78, kMakeupY + 78 + 21), kMakeup, &knob, &text, "dB"));
  pGraphics->AttachControl(new IKnobMultiControl(this, kDryWetX, kDryWetY, kDryWet, &knob1));
  
  AttachGraphics(pGraphics);
  
  //MakePreset("preset 1", ... );
  MakePreset("Serial Shaper", 10., 10., 10., 10., 100., 0., 2., .1, 0., .01, 0., 1.);
  MakePreset("Parallel Shaper", 10., 10., 10., 10., 100., 0., 2., .1, 0., .01, 0., 0.5);
  
  powerFilter.set_input_port(0, &inFilter, 0);
  slowAttackReleaseFilter.set_input_port(0, &powerFilter, 0);
  fastAttackReleaseFilter.set_input_port(0, &powerFilter, 0);
  invertFilter.set_input_port(0, &slowAttackReleaseFilter, 0);
  sumfilter.set_input_port(0, &invertFilter, 0);
  sumfilter.set_input_port(1, &fastAttackReleaseFilter, 0);
  gainCompressorFilter.set_input_port(0, &sumfilter, 0);
  applyGainFilter.set_input_port(0, &gainCompressorFilter, 0);
  applyGainFilter.set_input_port(1, &inFilter, 0);
  volumeFilter.set_input_port(0, &applyGainFilter, 0);
  drywetFilter.set_input_port(0, &volumeFilter, 0);
  drywetFilter.set_input_port(1, &inFilter, 0);
  outFilter.set_input_port(0, &drywetFilter, 0);
  
  invertFilter.set_volume(-1);
  
  Reset();
}

ATKTransientShaper::~ATKTransientShaper() {}

void ATKTransientShaper::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
  inFilter.set_pointer(inputs[0], nFrames);
  outFilter.set_pointer(outputs[0], nFrames);
  outFilter.process(nFrames);
}

void ATKTransientShaper::Reset()
{
  TRACE;
  IMutexLock lock(this);
  
  int sampling_rate = GetSampleRate();
  
  if(sampling_rate != outFilter.get_output_sampling_rate())
  {
    inFilter.set_input_sampling_rate(sampling_rate);
    inFilter.set_output_sampling_rate(sampling_rate);
    powerFilter.set_input_sampling_rate(sampling_rate);
    powerFilter.set_output_sampling_rate(sampling_rate);
    slowAttackReleaseFilter.set_input_sampling_rate(sampling_rate);
    slowAttackReleaseFilter.set_output_sampling_rate(sampling_rate);
    fastAttackReleaseFilter.set_input_sampling_rate(sampling_rate);
    fastAttackReleaseFilter.set_output_sampling_rate(sampling_rate);
    invertFilter.set_input_sampling_rate(sampling_rate);
    invertFilter.set_output_sampling_rate(sampling_rate);
    sumFilter.set_input_sampling_rate(sampling_rate);
    sumFilter.set_output_sampling_rate(sampling_rate);
    gainCompressorFilter.set_input_sampling_rate(sampling_rate);
    gainCompressorFilter.set_output_sampling_rate(sampling_rate);
    applyGainFilter.set_input_sampling_rate(sampling_rate);
    applyGainFilter.set_output_sampling_rate(sampling_rate);
    volumeFilter.set_input_sampling_rate(sampling_rate);
    volumeFilter.set_output_sampling_rate(sampling_rate);
    drywetFilter.set_input_sampling_rate(sampling_rate);
    drywetFilter.set_output_sampling_rate(sampling_rate);
    outFilter.set_input_sampling_rate(sampling_rate);
    outFilter.set_output_sampling_rate(sampling_rate);
    
    auto power = GetParam(kPower)->Value();
    if (power == 0)
    {
      powerFilter.set_memory(0);
    }
    else
    {
      powerFilter.set_memory(std::exp(-1e3 / (power * sampling_rate)));
    }
    slowAttackReleaseFilter.set_release(std::exp(-1e3/(GetParam(kRelease)->Value() * sampling_rate * GetParam(kReleaseRatio)->Value()))); // in ms
    slowAttackReleaseFilter.set_attack(std::exp(-1e3/(GetParam(kAttack)->Value() * sampling_rate))); // in ms
    fastAttackReleaseFilter.set_release(std::exp(-1e3/(GetParam(kRelease)->Value() * sampling_rate))); // in ms
    fastAttackReleaseFilter.set_attack(std::exp(-1e3/(GetParam(kAttack)->Value() * sampling_rate * GetParam(kAttackRatio)->Value()))); // in ms
  }
  
  powerFilter.full_setup();
  slowAttackReleaseFilter.full_setup();
  fastAttackReleaseFilter.full_setup();
}

void ATKTransientShaper::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  
  switch (paramIdx)
  {
    case kPower:
    {
      auto power = GetParam(kPower)->Value();
      if (power == 0)
      {
        powerFilter.set_memory(0);
      }
      else
      {
        powerFilter.set_memory(std::exp(-1e3 / (power * GetSampleRate())));
      }
      break;
    }
    case kThreshold:
      gainCompressorFilter.set_threshold(std::pow(10, GetParam(kThreshold)->Value() / 10));
      break;
    case kSlope:
      gainCompressorFilter.set_ratio(GetParam(kSlope)->Value());
      break;
    case kSoftness:
      gainCompressorFilter.set_softness(std::pow(10, GetParam(kSoftness)->Value()));
      break;
    case kColored:
      gainCompressorFilter.set_color(GetParam(kColored)->Value());
      break;
    case kQuality:
      gainCompressorFilter.set_quality(GetParam(kQuality)->Value());
      break;
    case kAttack:
      slowAttackReleaseFilter.set_attack(std::exp(-1e3/(GetParam(kAttack)->Value() * GetSampleRate()))); // in ms
      fastAttackReleaseFilter.set_attack(std::exp(-1e3/(GetParam(kAttack)->Value() * GetSampleRate() * GetParam(kAttackRatio)->Value()))); // in ms
      break;
    case kAttackRatio:
      fastAttackReleaseFilter.set_attack(std::exp(-1e3/(GetParam(kAttack)->Value() * GetSampleRate() * GetParam(kAttackRatio)->Value()))); // in ms
      break;
    case kRelease:
      fastAttackReleaseFilter.set_release(std::exp(-1e3/(GetParam(kRelease)->Value() * GetSampleRate()))); // in ms
      slowAttackReleaseFilter.set_release(std::exp(-1e3/(GetParam(kRelease)->Value() * GetSampleRate() * GetParam(kReleaseRatio)->Value()))); // in ms
      break;
    case kReleaseRatio:
      slowAttackReleaseFilter.set_release(std::exp(-1e3/(GetParam(kRelease)->Value() * GetSampleRate() * GetParam(kReleaseRatio)->Value()))); // in ms
      break;
    case kMakeup:
      volumeFilter.set_volume_db(GetParam(kMakeup)->Value());
      break;
    case kDryWet:
      drywetFilter.set_dry(GetParam(kDryWet)->Value());
      break;
      
    default:
      break;
  }
}
