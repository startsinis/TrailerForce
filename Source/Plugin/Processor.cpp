#include "Processor.h"
#include "Editor.h"
#include "Playback.h"
#include <cmath>
TrailerForceProcessor::TrailerForceProcessor()
 : AudioProcessor(BusesProperties()
 #if !JucePlugin_IsMidiEffect
 .withOutput("Output",juce::AudioChannelSet::stereo(),true)
 #endif
 ),parameters(*this,nullptr,"PARAMETERS",layout()),pending(std::make_unique<Playback>()),live(std::make_unique<Playback>()) { generate(config); }
juce::AudioProcessorValueTreeState::ParameterLayout TrailerForceProcessor::layout() {
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"monitor",1},"Internal sketch sound",true));
 p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gain",1},"Monitor level",juce::NormalisableRange<float>(0.f,1.f),.65f));
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"run",1},"Generate on DAW play",true));
 return p;
}
void TrailerForceProcessor::prepareToPlay(double rate,int) {sampleRateHz=rate;synth.prepare(rate);wasRunning=false;wasHost=false;internalBeat=0;}
bool TrailerForceProcessor::isBusesLayoutSupported(const BusesLayout& b) const {
 #if JucePlugin_IsMidiEffect
 return b.getMainInputChannelSet().isDisabled() && b.getMainOutputChannelSet().isDisabled();
 #else
 return b.getMainInputChannelSet().isDisabled() && (b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() || b.getMainOutputChannelSet()==juce::AudioChannelSet::mono());
 #endif
}
tf::Settings TrailerForceProcessor::settings() const {const juce::ScopedLock lock(mutex);return config;}
tf::Sequence TrailerForceProcessor::sequence() const {const juce::ScopedLock lock(mutex);return generated;}
void TrailerForceProcessor::generate(tf::Settings s,bool gesture) {
 s=tf::sanitise(s); auto seq=gesture?tf::soundGesture(s):tf::generate(s);auto ev=tf::events(seq);
 const juce::ScopedLock lock(mutex); config=s; generated=std::move(seq);
 pending->count=std::min(ev.size(),maxEvents);std::copy_n(ev.begin(),pending->count,pending->events.begin());
 pending->beats=generated.beats;pending->bpm=s.bpm;pending->sync=s.hostSync;pending->darkness=s.darkness;pending->motion=s.motion;pending->seconds=s.soundLength*60./s.bpm;
 pending->version=revision.fetch_add(1)+1;
}
void TrailerForceProcessor::allOff(juce::MidiBuffer& midi,int sample) {
 for(int ch=1;ch<=16;++ch) midi.addEvent(juce::MidiMessage::allNotesOff(ch),sample);
 synth.reset();
}
void TrailerForceProcessor::processBlock(juce::AudioBuffer<float>& audio,juce::MidiBuffer& midi) {
 juce::ScopedNoDenormals noDenormals;audio.clear();const int samples=audio.getNumSamples();if(samples<=0)return;
 bool replaced=false;
 if(revision.load()!=live->version) {
  const juce::ScopedTryLock lock(mutex);
  if(lock.isLocked()) {live->count=pending->count;std::copy_n(pending->events.begin(),pending->count,live->events.begin());live->beats=pending->beats;live->bpm=pending->bpm;live->sync=pending->sync;live->darkness=pending->darkness;live->motion=pending->motion;live->seconds=pending->seconds;live->version=pending->version;replaced=true;}
 }
 bool hp=false,havePpq=false;double ppq=0,tempo=live->bpm;
 if(auto* ph=getPlayHead()) if(auto pos=ph->getPosition()) {
  hp=pos->getIsPlaying();if(auto b=pos->getBpm())if(*b>0 && std::isfinite(*b))hostBpm.store(*b);
  if(auto p=pos->getPpqPosition()) {ppq=*p;havePpq=std::isfinite(ppq);}
 }
 hostPlaying.store(hp);
 bool hostRun=hp && parameters.getRawParameterValue("run")->load()>.5f;
 bool aud=audition.load();bool running=hostRun||aud;
 bool useHost=hostRun && live->sync && havePpq;
 if(useHost)tempo=hostBpm.load();tempo=juce::jlimit(20.,400.,tempo);
 const double beatsPerSample=tempo/(60.*sampleRateHz);
 bool jumped=useHost && wasHost && std::abs(ppq-expectedHostBeat)>std::max(.05,beatsPerSample*samples*2);
 int solo=soloLane.load();
 if(panicRequested.exchange(false)) {allOff(midi);running=false;}
 if(replaced || jumped || solo!=lastSolo || (wasRunning && !running) || (useHost!=wasHost)) allOff(midi);
 if((aud && !wasAudition && !hostRun) || (!wasRunning && running) || replaced)internalBeat=0;
 const double start=useHost?ppq:internalBeat;
 if(running && live->beats>0) {
  const double end=start+samples*beatsPerSample;
  tf::schedule(live->events.data(),live->count,live->beats,start,beatsPerSample,samples,solo,[&](const tf::Event& e,int offset){
    const uint8_t bytes[]{e.status,e.data1,e.data2};midi.addEvent(bytes,3,offset);
  });
  internalBeat=end;playBeat.store(std::fmod(std::fmod(start,live->beats)+live->beats,live->beats));
 } else playBeat.store(0);
 synth.configure(live->darkness,live->motion,live->seconds*live->bpm/tempo);
 #if !JucePlugin_IsMidiEffect
 bool monitor=parameters.getRawParameterValue("monitor")->load()>.5f;float gain=parameters.getRawParameterValue("gain")->load();
 auto it=midi.cbegin();auto finish=midi.cend();
 for(int i=0;i<samples;++i) {
  while(it!=finish && (*it).samplePosition<=i) {auto data=*it;if(data.numBytes>=3)synth.message(data.data[0],data.data[1],data.data[2]);++it;}
  auto sound=synth.sample();if(monitor)for(int ch=0;ch<audio.getNumChannels();++ch)audio.setSample(ch,i,sound[size_t(ch%2)]*gain);
 }
 #endif
 expectedHostBeat=ppq+samples*beatsPerSample;wasHost=useHost;wasRunning=running;wasAudition=aud;lastSolo=solo;
}
void TrailerForceProcessor::getStateInformation(juce::MemoryBlock& dest) {
 auto s=settings();juce::ValueTree root("TrailerForceState");root.setProperty("version",3,nullptr);root.addChild(parameters.copyState(),-1,nullptr);
 auto put=[&](const char* key,juce::var value){root.setProperty(key,value,nullptr);};
 put("brief",juce::String(s.brief));put("mood",juce::String(s.mood));put("instruments",juce::String(s.instruments));
 #define TF_SAVE(x) put(#x,s.x)
 TF_SAVE(style);TF_SAVE(key);TF_SAVE(mode);TF_SAVE(numerator);TF_SAVE(denominator);TF_SAVE(bpm);TF_SAVE(density);TF_SAVE(complexity);TF_SAVE(variation);TF_SAVE(humanize);put("seed",static_cast<juce::int64>(s.seed));TF_SAVE(breaks);TF_SAVE(button);TF_SAVE(hostSync);TF_SAVE(fullArrangement);TF_SAVE(selectedAct);TF_SAVE(selectedLane);TF_SAVE(patternBars);TF_SAVE(editEvery);TF_SAVE(atmosphere);TF_SAVE(motion);TF_SAVE(darkness);TF_SAVE(soundIntensity);TF_SAVE(soundLength);TF_SAVE(climax);TF_SAVE(soundType);TF_SAVE(harmony);TF_SAVE(groove);TF_SAVE(swing);TF_SAVE(gate);TF_SAVE(smartLayers);TF_SAVE(finalLift);TF_SAVE(expression);
 TF_SAVE(procedural);TF_SAVE(exploration);TF_SAVE(randomScope);
 put("generation",juce::String(std::to_string(s.generation)));
 for(int i=0;i<3;++i){root.setProperty("ideaSeed"+juce::String(i),static_cast<juce::int64>(s.ideaSeeds[size_t(i)]),nullptr);root.setProperty("ideaLock"+juce::String(i),s.ideaLocks[size_t(i)],nullptr);}
 #undef TF_SAVE
 for(int i=0;i<4;++i)root.setProperty("act"+juce::String(i),s.bars[size_t(i)],nullptr);
 for(int i=0;i<tf::laneCount;++i)root.setProperty("lane"+juce::String(i),s.enabled[size_t(i)],nullptr);
 if(auto xml=root.createXml())copyXmlToBinary(*xml,dest);
}
void TrailerForceProcessor::setStateInformation(const void* data,int bytes) {
 auto xml=getXmlFromBinary(data,bytes);if(!xml || !xml->hasTagName("TrailerForceState"))return;
 auto root=juce::ValueTree::fromXml(*xml);auto s=tf::Settings{};
 #define TF_LOAD(x,type) s.x=static_cast<type>(root.getProperty(#x,s.x))
 TF_LOAD(style,int);TF_LOAD(key,int);TF_LOAD(mode,int);TF_LOAD(numerator,int);TF_LOAD(denominator,int);TF_LOAD(bpm,double);TF_LOAD(density,double);TF_LOAD(complexity,double);TF_LOAD(variation,double);TF_LOAD(humanize,double);s.seed=uint32_t(static_cast<juce::int64>(root.getProperty("seed",42)));TF_LOAD(breaks,bool);TF_LOAD(button,bool);TF_LOAD(hostSync,bool);TF_LOAD(fullArrangement,bool);TF_LOAD(selectedAct,int);TF_LOAD(selectedLane,int);TF_LOAD(patternBars,int);TF_LOAD(editEvery,int);TF_LOAD(atmosphere,double);TF_LOAD(motion,double);TF_LOAD(darkness,double);TF_LOAD(soundIntensity,double);TF_LOAD(soundLength,double);TF_LOAD(climax,double);TF_LOAD(soundType,int);TF_LOAD(harmony,int);TF_LOAD(groove,int);TF_LOAD(swing,double);TF_LOAD(gate,double);TF_LOAD(smartLayers,bool);TF_LOAD(finalLift,bool);TF_LOAD(expression,bool);
 TF_LOAD(exploration,double);TF_LOAD(randomScope,int);
 s.procedural=bool(root.getProperty("procedural",false));
 s.generation=root.getProperty("generation","0").toString().getLargeIntValue();
 for(int i=0;i<3;++i){s.ideaSeeds[size_t(i)]=uint32_t(static_cast<juce::int64>(root.getProperty("ideaSeed"+juce::String(i),static_cast<juce::int64>(s.ideaSeeds[size_t(i)]))));s.ideaLocks[size_t(i)]=bool(root.getProperty("ideaLock"+juce::String(i),false));}
 #undef TF_LOAD
 s.brief=root.getProperty("brief",juce::String(s.brief)).toString().toStdString();s.mood=root.getProperty("mood",juce::String(s.mood)).toString().toStdString();s.instruments=root.getProperty("instruments",juce::String(s.instruments)).toString().toStdString();
 for(int i=0;i<4;++i)s.bars[size_t(i)]=int(root.getProperty("act"+juce::String(i),s.bars[size_t(i)]));
 for(int i=0;i<tf::laneCount;++i)s.enabled[size_t(i)]=bool(root.getProperty("lane"+juce::String(i),s.enabled[size_t(i)]));
 auto param=root.getChildWithName("PARAMETERS");if(param.isValid())parameters.replaceState(param);
 generate(s);panicRequested.store(true);
}
juce::AudioProcessorEditor* TrailerForceProcessor::createEditor(){return new TrailerForceEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new TrailerForceProcessor();}
