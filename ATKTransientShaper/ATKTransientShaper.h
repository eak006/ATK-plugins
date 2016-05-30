#ifndef __ATKTransientShaper__
#define __ATKTransientShaper__

#include "IPlug_include_in_plug_hdr.h"
#include "controls.h"

#include <ATK/Core/InPointerFilter.h>
#include <ATK/Core/OutPointerFilter.h>

#include <ATK/Dynamic/AttackReleaseFilter.h>
#include <ATK/Dynamic/GainColoredCompressorFilter.h>
#include <ATK/Dynamic/PowerFilter.h>

#include <ATK/Tools/ApplyGainFilter.h>
#include <ATK/Tools/DryWetFilter.h>
#include <ATK/Tools/SumFilter.h>
#include <ATK/Tools/VolumeFilter.h>

class ATKTransientShaper : public IPlug
{
public:
  ATKTransientShaper(IPlugInstanceInfo instanceInfo);
  ~ATKTransientShaper();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  ATK::InPointerFilter<double> inFilter;
  ATK::PowerFilter<double> powerFilter;
  ATK::AttackReleaseFilter<double> slowAttackReleaseFilter;
  ATK::AttackReleaseFilter<double> fastAttackReleaseFilter;
  ATK::VolumeFilter<double> invertFilter;
  ATK::SumFilter<double> sumFilter;
  ATK::GainColoredCompressorFilter<double> gainCompressorFilter;
  ATK::ApplyGainFilter<double> applyGainFilter;
  ATK::VolumeFilter<double> volumeFilter;
  ATK::DryWetFilter<double> drywetFilter;
  ATK::OutPointerFilter<double> outFilter;
};

#endif
