#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include "Sound.h"
#include <atomic>
class TrailerForceProcessor final : public juce::AudioProcessor {
public:
 TrailerForceProcessor();
 const juce::String getName() const override { return JucePlugin_Name; }
 void prepareToPlay(double,int) override;
 void releaseResources() override { synth.reset(); }
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 bool acceptsMidi() const override {return true;}
 bool producesMidi() const override {return true;}
 bool isMidiEffect() const override {return JucePlugin_IsMidiEffect;}
 double getTailLengthSeconds() const override {return 1.5;}
 bool hasEditor() const override {return true;}
 juce::AudioProcessorEditor* createEditor() override;
 int getNumPrograms() override {return 1;} int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {} const juce::String getProgramName(int) override {return "Trailer Force";}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 tf::Settings settings() const;
 tf::Sequence sequence() const;
 void generate(tf::Settings, bool gesture=false);
 void setAudition(bool value) { audition.store(value); }
 bool isAuditioning() const {return audition.load();}
 void panic() {panicRequested.store(true);audition.store(false);}
 std::atomic<double> playBeat{0},hostBpm{120};
 std::atomic<bool> hostPlaying{false};
 std::atomic<int> soloLane{-1};
 std::atomic<uint64_t> revision{0};
 juce::AudioProcessorValueTreeState parameters;
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 static constexpr size_t maxEvents=131072;
 struct Playback {
  std::array<tf::Event,maxEvents> events{}; size_t count=0;
  double beats=16,bpm=120,darkness=.6,motion=.3,seconds=2;
  bool sync=true; uint64_t version=0;
 };
 mutable juce::CriticalSection mutex;
 tf::Settings config; tf::Sequence generated;
 std::unique_ptr<Playback> pending,live;
 tf::Sound synth;
 std::atomic<bool> audition{false},panicRequested{false};
 double sampleRateHz=44100,internalBeat=0,expectedHostBeat=0;
 bool wasRunning=false,wasAudition=false,wasHost=false;
 int lastSolo=-1;
 void allOff(juce::MidiBuffer&,int sample=0);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrailerForceProcessor)
};
