#define PLUG_MFR "MatthieuBrucher"
#define PLUG_NAME "ATKSD1"

#define PLUG_CLASS_NAME ATKSD1

#define BUNDLE_MFR "MatthieuBrucher"
#define BUNDLE_NAME "ATKSD1"

#define PLUG_ENTRY ATKSD1_Entry
#define PLUG_VIEW_ENTRY ATKSD1_ViewEntry

#define PLUG_ENTRY_STR "ATKSD1_Entry"
#define PLUG_VIEW_ENTRY_STR "ATKSD1_ViewEntry"

#define VIEW_CLASS ATKSD1_View
#define VIEW_CLASS_STR "ATKSD1_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VER 0x00020000
#define VST3_VER_STR "2.0.0"

// http://service.steinberg.de/databases/plugin.nsf/plugIn?openForm
// 4 chars, single quotes. At least one capital letter
#define PLUG_UNIQUE_ID 'Jh5n'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'MatB'

// ProTools stuff

#if (defined(AAX_API) || defined(RTAS_API)) && !defined(_PIDS_)
  #define _PIDS_
  const int PLUG_TYPE_IDS[2] = {'EFN1', 'EFN2'};
  const int PLUG_TYPE_IDS_AS[2] = {'EFA1', 'EFA2'}; // AudioSuite
#endif

#define PLUG_MFR_PT "MatthieuBrucher\nMatthieuBrucher\nMatthieuBrucher"
#define PLUG_NAME_PT "ATKSD1\nJh5n"
#define PLUG_TYPE_PT "Effect"
#define PLUG_DOES_AUDIOSUITE 1

/* PLUG_TYPE_PT can be "None", "EQ", "Dynamics", "PitchShift", "Reverb", "Delay", "Modulation", 
"Harmonic" "NoiseReduction" "Dither" "SoundField" "Effect" 
instrument determined by PLUG _IS _INST
*/

#define PLUG_CHANNEL_IO "1-1"

// Should be retrieved from ATK when the time comes
#define PLUG_LATENCY 3
#define PLUG_IS_INST 0

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 0

#define PLUG_DOES_STATE_CHUNKS 0

// Unique IDs for each image resource.
#define SD1_ID 101
#define BIGKNOB_ID 102
#define SMALLKNOB_ID 103

// Image resource locations for this plug.
#define SD1_FN "resources/img/Background.png"
#define SMALLKNOB_FN "resources/img/SmallKnob.png"
#define BIGKNOB_FN "resources/img/BigKnob.png"

// GUI default dimensions
#define GUI_WIDTH 400
#define GUI_HEIGHT 200

// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#if defined(SA_API) && !defined(OS_IOS)
#include "app_wrapper/app_resource.h"
#endif

// vst3 stuff
#define MFR_URL "matt.eifelle.com"
#define MFR_EMAIL "spam@me.com"
#define EFFECT_TYPE_VST3 "Fx|Distortion"

/* "Fx|Analyzer"", "Fx|Delay", "Fx|Distortion", "Fx|Dynamics", "Fx|EQ", "Fx|Filter",
"Fx", "Fx|Instrument", "Fx|InstrumentExternal", "Fx|Spatial", "Fx|Generator",
"Fx|Mastering", "Fx|Modulation", "Fx|PitchShift", "Fx|Restoration", "Fx|Reverb",
"Fx|Surround", "Fx|Tools", "Instrument", "Instrument|Drum", "Instrument|Sampler",
"Instrument|Synth", "Instrument|Synth|Sampler", "Instrument|External", "Spatial",
"Spatial|Fx", "OnlyRT", "OnlyOfflineProcess", "Mono", "Stereo",
"Surround"
*/
