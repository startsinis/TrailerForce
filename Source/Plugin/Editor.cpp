#include "Editor.h"
#include "Profiles.h"
namespace { const juce::Colour bg{0xff111823},panel{0xff1c2837},accent{0xffa9d9ee},sage{0xffa9c9b0},ink{0xffe7eef4},muted{0xff9baabb}; }
void Roll::paint(juce::Graphics& g) {
 g.fillAll(bg.brighter(.04f));auto r=getLocalBounds().reduced(10);g.setColour(muted);g.setFont(12.f);g.drawText("MIDI OVERVIEW  /  "+juce::String(lane<0?"All lanes":tf::laneNames[size_t(lane)]),r.removeFromTop(22),juce::Justification::centredLeft);
 if(sequence.beats<=0)return;
 double bpb=sequence.numerator*4./sequence.denominator;
 for(double b=0;b<=sequence.beats;b+=bpb){float x=float(r.getX()+r.getWidth()*b/sequence.beats);g.setColour(panel.brighter(.1f));g.drawVerticalLine(int(x),float(r.getY()),float(r.getBottom()));}
 for(auto& m:sequence.markers)if(m.text.rfind("ACT",0)==0){int x=r.getX()+int(r.getWidth()*m.beat/sequence.beats);g.setColour(sage);g.drawText(m.text,juce::Rectangle<int>{x+3,r.getY(),190,16},juce::Justification::centredLeft);}
 auto notesArea=r.withTrimmedTop(22);
 for(auto& n:sequence.notes)if(lane<0 || n.lane==lane){float x=float(notesArea.getX()+notesArea.getWidth()*n.beat/sequence.beats);float w=float(std::max(2.,notesArea.getWidth()*n.length/sequence.beats));float y=float(notesArea.getBottom()-(n.pitch-12)/100.*notesArea.getHeight());g.setColour(juce::Colour::fromHSV(float(n.lane)/12.f,.35f,.95f,float(n.velocity)/150.f+.15f));g.fillRoundedRectangle(x,y,w,3.5f,1.f);}
 g.setColour(accent);int x=r.getX()+int(r.getWidth()*beat/sequence.beats);g.drawVerticalLine(x,float(r.getY()),float(r.getBottom()));
}
void TrailerForceEditor::label(int page,juce::String value,juce::Rectangle<int> b){auto l=std::make_unique<juce::Label>();l->setText(value,juce::dontSendNotification);l->setColour(juce::Label::textColourId,muted);forms[size_t(page)]->add(std::move(l),b);}
void TrailerForceEditor::text(int page,juce::String value,juce::Rectangle<int> b){auto l=std::make_unique<juce::Label>();l->setText(value,juce::dontSendNotification);l->setColour(juce::Label::textColourId,ink);l->setJustificationType(juce::Justification::topLeft);l->setMinimumHorizontalScale(1.f);forms[size_t(page)]->add(std::move(l),b);}
void TrailerForceEditor::slider(int page,juce::String name,juce::Rectangle<int> b,double min,double max,double step,std::function<double()> get,std::function<void(double)> set){
 label(page,name,b.withHeight(22));auto c=std::make_unique<juce::Slider>();c->setSliderStyle(juce::Slider::LinearHorizontal);c->setTextBoxStyle(juce::Slider::TextBoxRight,false,65,25);c->setRange(min,max,step);auto* raw=forms[size_t(page)]->add(std::move(c),b.withTrimmedTop(24));raw->onValueChange=[this,raw,set]{if(!refreshing){set(raw->getValue());changed();}};reload.push_back([raw,get]{raw->setValue(get(),juce::dontSendNotification);});
}
void TrailerForceEditor::combo(int page,juce::String name,juce::Rectangle<int> b,juce::StringArray items,std::function<int()> get,std::function<void(int)> set){
 label(page,name,b.withHeight(22));auto c=std::make_unique<juce::ComboBox>();c->addItemList(items,1);auto* raw=forms[size_t(page)]->add(std::move(c),b.withTrimmedTop(24));raw->onChange=[this,raw,set]{if(!refreshing){set(raw->getSelectedItemIndex());changed();}};reload.push_back([raw,get]{raw->setSelectedItemIndex(get(),juce::dontSendNotification);});
}
void TrailerForceEditor::toggle(int page,juce::String name,juce::Rectangle<int> b,std::function<bool()> get,std::function<void(bool)> set){
 auto c=std::make_unique<juce::ToggleButton>(name);auto* raw=forms[size_t(page)]->add(std::move(c),b);raw->onClick=[this,raw,set]{if(!refreshing){set(raw->getToggleState());changed();}};reload.push_back([raw,get]{raw->setToggleState(get(),juce::dontSendNotification);});
}
TrailerForceEditor::TrailerForceEditor(TrailerForceProcessor& processor):AudioProcessorEditor(processor),p(processor),working(p.settings()) {
 look.setColour(juce::ResizableWindow::backgroundColourId,bg);look.setColour(juce::Label::textColourId,ink);look.setColour(juce::TextButton::buttonColourId,panel);look.setColour(juce::TextButton::textColourOffId,ink);look.setColour(juce::ComboBox::backgroundColourId,panel);look.setColour(juce::ComboBox::textColourId,ink);look.setColour(juce::PopupMenu::backgroundColourId,panel);look.setColour(juce::PopupMenu::textColourId,ink);look.setColour(juce::Slider::trackColourId,sage);look.setColour(juce::Slider::thumbColourId,accent);look.setColour(juce::TextEditor::backgroundColourId,bg);look.setColour(juce::TextEditor::textColourId,ink);look.setColour(juce::TextEditor::outlineColourId,panel.brighter(.2f));look.setColour(juce::TabbedButtonBar::tabTextColourId,ink);setLookAndFeel(&look);
 const char* names[]{"Creative Brief","MIDI Designer","Structure","Atmosphere","Sound Design","Climax","What Next?","Production","Generative"};
 for(int i=0;i<9;++i){forms[size_t(i)]=new Form();tabs.addTab(names[i],panel,forms[size_t(i)],true);}addAndMakeVisible(tabs);tabs.setTabBarDepth(38);
 for(juce::Component* c:std::initializer_list<juce::Component*>{&status,&transport,&generateButton,&variationButton,&auditionButton,&stopButton,&saveButton,&monitor,&follow,&solo,&gain,&lane,&drag,&roll})addAndMakeVisible(c);
 generateButton.setColour(juce::TextButton::buttonColourId,accent);generateButton.setColour(juce::TextButton::textColourOffId,bg);
 monitorAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"monitor",monitor);followAttachment=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters,"run",follow);
 gain.setSliderStyle(juce::Slider::LinearHorizontal);gain.setTextBoxStyle(juce::Slider::TextBoxRight,false,45,25);gainAttachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,"gain",gain);
 #if JucePlugin_IsMidiEffect
 monitor.setVisible(false);gain.setVisible(false);
 #endif
 lane.addItem("All lanes / multitrack",1);for(int i=0;i<tf::laneCount;++i)lane.addItem(tf::laneNames[size_t(i)],i+2);lane.setSelectedId(1);lane.onChange=[this]{roll.lane=lane.getSelectedId()-2;p.soloLane.store(solo.getToggleState()?roll.lane:-1);roll.repaint();};solo.onClick=[this]{p.soloLane.store(solo.getToggleState()?roll.lane:-1);};
 generateButton.onClick=[this]{commit();};variationButton.onClick=[this]{previousVariation=working;hasPreviousVariation=true;working=tf::newVariation(working);commit();sync();};auditionButton.onClick=[this]{if(dirty)commit();p.setAudition(!p.isAuditioning());};stopButton.onClick=[this]{p.panic();if(auto* param=p.parameters.getParameter("run"))param->setValueNotifyingHost(0.f);};saveButton.onClick=[this]{exportMidi(false);};drag.beginDrag=[this]{exportMidi(true);};
 label(0,"Describe the cue. Include genre, mood, key, tempo and act lengths.",{20,8,1000,26});
 auto box=std::make_unique<juce::TextEditor>();box->setMultiLine(true);box->setReturnKeyStartsNewLine(true);box->setFont(juce::Font(juce::FontOptions(17.f)));box->setInputRestrictions(16000);box->setTextToShowWhenEmpty("Example: Dark hybrid in D minor, 120 BPM. Act one: 4 bars, act two: 8 bars, act three: 8 bars, act four: 4 bars. Strings, brass and metal. Button ending.",muted);brief=forms[0]->add(std::move(box),{20,40,1020,132});brief->onTextChange=[this]{if(!refreshing){working.brief=brief->getText().toStdString();changed();}};
 auto interpretButton=std::make_unique<juce::TextButton>("INTERPRET BRIEF + GENERATE");interpretButton->onClick=[this]{auto result=tf::parseBrief(brief->getText().toStdString(),working);working=result.settings;interpretation->setText(result.report,false);commit();sync();};forms[0]->add(std::move(interpretButton),{20,182,290,34});
 juce::StringArray styleNames;for(auto n:tf::styles)styleNames.add(n);
 combo(0,"Style preset",{340,177,350,64},styleNames,[this]{return working.style;},[this](int v){auto old=working;working=tf::preset(v);working.brief=old.brief;working.seed=old.seed;sync();});
 auto report=std::make_unique<juce::TextEditor>();report->setMultiLine(true);report->setReadOnly(true);interpretation=forms[0]->add(std::move(report),{20,250,1020,118});
 const juce::StringArray keys{"C","C# / Db","D","D# / Eb","E","F","F# / Gb","G","G# / Ab","A","A# / Bb","B"};
 combo(1,"Key",{20,15,220,62},keys,[this]{return working.key;},[this](int v){working.key=v;});
 combo(1,"Mode",{275,15,235,62},{"Minor","Major","Dorian","Phrygian","Harmonic minor"},[this]{return working.mode;},[this](int v){working.mode=v;});
 slider(1,"Tempo (BPM)",{545,15,230,62},30,240,.1,[this]{return working.bpm;},[this](double v){working.bpm=v;});
 toggle(1,"Sync to host BPM",{805,39,235,35},[this]{return working.hostSync;},[this](bool v){working.hostSync=v;});
 slider(1,"Density",{20,100,235,62},0,1,.01,[this]{return working.density;},[this](double v){working.density=v;});
 slider(1,"Complexity",{285,100,235,62},0,1,.01,[this]{return working.complexity;},[this](double v){working.complexity=v;});
 slider(1,"Variation",{550,100,235,62},0,1,.01,[this]{return working.variation;},[this](double v){working.variation=v;});
 slider(1,"Humanize",{815,100,225,62},0,1,.01,[this]{return working.humanize;},[this](double v){working.humanize=v;});
 slider(1,"Pattern bars",{20,185,235,62},1,16,1,[this]{return working.patternBars;},[this](double v){working.patternBars=int(v);});
 combo(1,"Pattern energy",{285,185,235,62},{"Act I / setup","Act II / build","Act III / climax","Act IV / final lift"},[this]{return working.selectedAct;},[this](int v){working.selectedAct=v;});
 toggle(1,"Generate full arrangement",{550,209,300,35},[this]{return working.fullArrangement;},[this](bool v){working.fullArrangement=v;});
 for(int i=0;i<tf::laneCount;++i)toggle(1,tf::laneNames[size_t(i)],{20+(i%5)*208,275+(i/5)*42,200,32},[this,i]{return working.enabled[size_t(i)];},[this,i](bool v){working.enabled[size_t(i)]=v;});
 for(int i=0;i<4;++i)slider(2,"Act "+juce::String(i+1)+" / bars",{20+i*260,20,235,64},1,32,1,[this,i]{return working.bars[size_t(i)];},[this,i](double v){working.bars[size_t(i)]=int(v);});
 combo(2,"Time signature",{20,110,235,64},{"3/4","4/4","5/4","6/8","7/8","9/8","12/8"},[this]{const int n[]{3,4,5,6,7,9,12};const int d[]{4,4,4,8,8,8,8};for(int i=0;i<7;++i)if(n[i]==working.numerator && d[i]==working.denominator)return i;return -1;},[this](int v){const int n[]{3,4,5,6,7,9,12};working.numerator=n[v];working.denominator=v<3?4:8;});
 slider(2,"Edit point every / bars",{280,110,235,64},1,16,1,[this]{return working.editEvery;},[this](double v){working.editEvery=int(v);});
 toggle(2,"Break before edit points",{560,135,240,35},[this]{return working.breaks;},[this](bool v){working.breaks=v;});
 toggle(2,"Button ending",{820,135,210,35},[this]{return working.button;},[this](bool v){working.button=v;});
 text(2,"I  SETUP: sparse hook and atmosphere\nII  BUILD: rhythmic foundation and tension\nIII  CLIMAX: density, accents and layered motion\nIV  FINAL LIFT / RESOLUTION: choose the second climax in Production\n\nMIDI exports include act, edit, break, transition and button markers. The final bar starts with the button hit when enabled. Breaks cut MIDI note lengths; sample-library release tails remain under your instrument's control.",{20,215,1020,156});
 slider(3,"Atmosphere level / velocity",{20,25,310,64},0,1,.01,[this]{return working.atmosphere;},[this](double v){working.atmosphere=v;});
 slider(3,"Motion",{375,25,310,64},0,1,.01,[this]{return working.motion;},[this](double v){working.motion=v;});
 slider(3,"Darkness",{730,25,310,64},0,1,.01,[this]{return working.darkness;},[this](double v){working.darkness=v;});
 text(3,"ATMOSPHERE DESIGNER\n\nGenerate root drones, open-fifth beds and moving upper notes. Higher Motion introduces repeated upper notes; lower Darkness opens the voicing.\n\nSelect Atmosphere below, enable Solo, then audition or drag the MIDI into a pad, granular synth or bowed-texture instrument.\n\nThe internal monitor uses a slowly swelling layered oscillator. Darkness and Motion also shape its tone and modulation. These controls do not change external instruments automatically.",{20,130,1020,225});
 juce::StringArray sounds;for(auto n:tf::soundNames)sounds.add(n);
 combo(4,"Gesture",{20,20,250,64},sounds,[this]{return working.soundType;},[this](int v){working.soundType=v;});
 slider(4,"Intensity",{310,20,330,64},0,1,.01,[this]{return working.soundIntensity;},[this](double v){working.soundIntensity=v;});
 slider(4,"Length / quarter-note beats",{680,20,360,64},.25,16,.25,[this]{return working.soundLength;},[this](double v){working.soundLength=v;});
 auto gesture=std::make_unique<juce::TextButton>("GENERATE ISOLATED GESTURE");gesture->onClick=[this]{commit(true);lane.setSelectedId(tf::Design+2);p.setAudition(true);};forms[4]->add(std::move(gesture),{20,110,370,36});
 
 auto render=std::make_unique<juce::TextButton>("EXPORT GESTURE WAV");render->onClick=[this]{
  auto s=working;auto seq=tf::soundGesture(s);auto bytes=std::make_shared<std::vector<uint8_t>>(tf::renderWave(seq,s,tf::Design));
  chooser=std::make_unique<juce::FileChooser>("Save generated gesture",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("TrailerForce-"+juce::String(tf::soundNames[size_t(s.soundType)])+".wav"),"*.wav");
  juce::Component::SafePointer<TrailerForceEditor> safe(this);
  chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,[safe,bytes](const juce::FileChooser& fc){if(!safe)return;auto f=fc.getResult();if(f==juce::File{})return;bool ok=f.replaceWithData(bytes->data(),bytes->size());safe->status.setText(ok?"WAV saved: "+f.getFileName():"Could not save WAV.",juce::dontSendNotification);});
 };forms[4]->add(std::move(render),{420,110,330,36});
 text(4,"SOUND DESIGN DIRECTOR\n\nBraam: low detuned saturation. Impact: falling sub plus noise transient. Riser: ascending tone and noise. Downer: descending pitch. Whoosh: shaped filtered noise. Signature: metallic FM with rhythmic modulation.\n\nThe six gestures are synthesised, not text-only suggestions. Export a 44.1 kHz stereo WAV, or MIDI trigger notes on channel 8: 24 / 36 / 48 / 60 / 72 / 84. Map those notes in your sampler.\n\nGenerate MIDI returns to your full arrangement; isolated gesture mode is an audition scratchpad.",{20,177,1020,196});
 slider(5,"Act III climax intensity",{20,25,480,70},0,1,.01,[this]{return working.climax;},[this](double v){working.climax=v;});
 auto build=std::make_unique<juce::TextButton>("BUILD CLIMAX LAYERS");build->onClick=[this]{working.enabled.fill(true);working.selectedAct=2;working.fullArrangement=false;working.density=std::max(.7,working.density);working.complexity=std::max(.65,working.complexity);working.climax=1.;commit();sync();};forms[5]->add(std::move(build),{560,52,400,40});
 text(5,"CLIMAX BUILDER\n\nBuild Climax Layers creates a focused Act III pattern with all nine lanes enabled. Set Pattern Bars in MIDI Designer, then export each lane to a dedicated instrument.\n\n1  Bass and percussion establish the low-end rhythm.\n2  Ostinato and pulse add motion.\n3  Motif and chords carry the hook and harmony.\n4  Atmosphere and sound design widen the arrangement.\n5  Transition notes accelerate into the edit.\n\nEnable Full Arrangement again to place this energy back into the four-act structure.",{20,140,1020,220});
 auto next=std::make_unique<juce::TextEditor>();next->setMultiLine(true);next->setReadOnly(true);next->setFont(juce::Font(juce::FontOptions(16.f)));guide=forms[6]->add(std::move(next),{20,20,1020,350});
 combo(7,"Harmonic palette / scale degrees",{20,10,350,64},{"Genre default","1 - 6 - 3 - 7","1 - 5 - 6 - 4","1 - 4 - 6 - 5","Pedal: 1 - 1 - 6 - 1","1 - 7 - 6 - 7"},[this]{return working.harmony;},[this](int v){working.harmony=v;});
 combo(7,"Groove",{410,10,280,64},{"Genre pocket","Straight backbeat","Half-time backbeat","Triplet subdivision"},[this]{return working.groove;},[this](int v){working.groove=v;});
 slider(7,"Short-note gate",{730,10,310,64},.2,1.2,.01,[this]{return working.gate;},[this](double v){working.gate=v;});
 slider(7,"Swing / offbeat delay",{20,90,350,64},0,.45,.01,[this]{return working.swing;},[this](double v){working.swing=v;});
 toggle(7,"Stage layer entrances",{410,112,250,36},[this]{return working.smartLayers;},[this](bool v){working.smartLayers=v;});
 toggle(7,"Act IV: second climax",{730,112,310,36},[this]{return working.finalLift;},[this](bool v){working.finalLift=v;});
 toggle(7,"Export / send CC1 + CC11 (Chords and Atmosphere)",{20,174,700,34},[this]{return working.expression;},[this](bool v){working.expression=v;});
 auto reference=std::make_unique<juce::TextButton>("OPEN GENRE REFERENCE");reference->onClick=[this]{juce::URL(tf::profile(working.style).source).launchInDefaultBrowser();};forms[7]->add(std::move(reference),{730,174,310,34});
 toggle(8,"Procedural composition",{20,20,300,30},[this]{return working.procedural;},[this](bool v){working.procedural=v;});
 slider(8,"Exploration: distance from genre vocabulary",{20,65,480,64},0,1,.01,[this]{return working.exploration;},[this](double v){working.exploration=v;});
 combo(8,"New variation changes",{540,65,480,64},{"All unlocked ideas","Melody + ostinato","Harmony","Rhythm + bass attacks"},[this]{return working.randomScope;},[this](int v){working.randomScope=v;});
 for(int i=0;i<3;++i){const char* locks[]{"Lock melodic design","Lock harmony design","Lock rhythm design"};toggle(8,locks[i],{20+i*340,150,320,30},[this,i]{return working.ideaLocks[size_t(i)];},[this,i](bool v){working.ideaLocks[size_t(i)]=v;});}
 auto undo=std::make_unique<juce::TextButton>("A/B: PREVIOUS VARIATION");undo->onClick=[this]{if(hasPreviousVariation){std::swap(working,previousVariation);commit();sync();}};forms[8]->add(std::move(undo),{20,195,400,34});
 auto brand=std::make_unique<juce::TextButton>("BRAND X MUSIC CATALOG");brand->onClick=[]{juce::URL("https://brandxmusic.bandcamp.com/album/chronos").launchInDefaultBrowser();};forms[8]->add(std::move(brand),{540,195,480,34});
 text(8,"NEW VARIATION composes connected melodic calls and answers, chord journeys and two-bar grooves. Key, tempo, acts and edit points stay fixed. Locks preserve a design during randomization; changing harmony can still transpose the ostinato and bass. Explicit harmony palettes override generated roots.\n\nA/B compares the last two variations while this editor stays open. Designs and locks save with your DAW session. Large procedural variety, not guaranteed infinite unique results. Brand X catalog is a reference resource; no recordings or melodies are copied, and no affiliation is implied.",{20,245,1020,140});
 auto productionBox=std::make_unique<juce::TextEditor>();productionBox->setMultiLine(true);productionBox->setReadOnly(true);production=forms[7]->add(std::move(productionBox),{20,224,1020,154});
 setResizable(true,true);setResizeLimits(1120,850,1800,1200);setSize(1180,900);sync();commit();startTimerHz(20);
}
TrailerForceEditor::~TrailerForceEditor(){stopTimer();if(dirty)commit();p.soloLane.store(-1);setLookAndFeel(nullptr);}
void TrailerForceEditor::changed(){dirty=true;lastEdit=juce::Time::getMillisecondCounterHiRes();}
void TrailerForceEditor::commit(bool gesture){working=tf::sanitise(working);p.generate(working,gesture);dirty=false;seenRevision=p.revision.load();roll.sequence=p.sequence();guide->setText(tf::nextStep(working,roll.sequence),false);production->setText(tf::productionNotes(working),false);status.setText(juce::String(static_cast<int>(roll.sequence.notes.size()))+" notes  |  "+juce::String(roll.sequence.beats*60./working.bpm,1)+" seconds at "+juce::String(working.bpm,1)+" BPM  |  Variation "+juce::String(std::to_string(working.generation)),juce::dontSendNotification);roll.repaint();}
void TrailerForceEditor::sync(){refreshing=true;for(auto& f:reload)f();brief->setText(working.brief,false);refreshing=false;}
void TrailerForceEditor::timerCallback(){
 if(dirty && juce::Time::getMillisecondCounterHiRes()-lastEdit>220)commit();
 if(!dirty && p.revision.load()!=seenRevision){working=p.settings();seenRevision=p.revision.load();sync();roll.sequence=p.sequence();guide->setText(tf::nextStep(working,roll.sequence),false);production->setText(tf::productionNotes(working),false);}
 roll.beat=p.playBeat.load();roll.repaint();auditionButton.setButtonText(p.isAuditioning()?"STOP AUDITION":"AUDITION");transport.setText(juce::String(p.hostPlaying.load()?"DAW PLAYING":"DAW STOPPED")+"  /  "+juce::String(p.hostBpm.load(),1)+" BPM",juce::dontSendNotification);
}
void TrailerForceEditor::exportMidi(bool dragging){
 if(dirty)commit();auto bytes=std::make_shared<std::vector<uint8_t>>(tf::midiFile(p.sequence(),lane.getSelectedId()-2));
 if(dragging){
  auto folder=juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("VinciSounds-TrailerForce");if(folder.createDirectory().failed()){status.setText("Could not create MIDI export folder.",juce::dontSendNotification);return;}
  dragFile=folder.getNonexistentChildFile("TrailerForce-"+lane.getText().replaceCharacter('/','-'),".mid");
  if(!dragFile.replaceWithData(bytes->data(),bytes->size())){status.setText("Could not write drag MIDI file.",juce::dontSendNotification);return;}
  if(!juce::DragAndDropContainer::performExternalDragDropOfFiles({dragFile.getFullPathName()},false,&drag))status.setText("DAW did not accept the drag. Use Save MIDI, then import.",juce::dontSendNotification);
  return;
 }
 chooser=std::make_unique<juce::FileChooser>("Save editable MIDI",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("TrailerForce.mid"),"*.mid");juce::Component::SafePointer<TrailerForceEditor> safe(this);
 chooser->launchAsync(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting,[safe,bytes](const juce::FileChooser& fc){if(!safe)return;auto f=fc.getResult();if(f==juce::File{})return;bool ok=f.replaceWithData(bytes->data(),bytes->size());safe->status.setText(ok?"MIDI saved: "+f.getFileName():"Could not save MIDI.",juce::dontSendNotification);});
}
void TrailerForceEditor::paint(juce::Graphics& g){g.fillAll(bg);g.setColour(accent);g.setFont(29.f);g.drawText("TRAILER FORCE",24,12,370,38,juce::Justification::centredLeft);g.setColour(sage);g.setFont(13.f);g.drawText("BY VINCI SOUNDS  /  NATIVE COMPOSING PARTNER",26,51,520,22,juce::Justification::centredLeft);}
void TrailerForceEditor::resized(){
 int w=getWidth();transport.setBounds(w-410,20,385,25);monitor.setBounds(w-415,51,130,27);gain.setBounds(w-280,51,125,27);follow.setBounds(w-145,51,125,27);
 tabs.setBounds(20,88,w-40,435);int y=537;
 generateButton.setBounds(25,y,200,38);variationButton.setBounds(235,y,185,38);auditionButton.setBounds(435,y,160,38);stopButton.setBounds(610,y,165,38);
 lane.setBounds(25,y+52,285,33);solo.setBounds(330,y+52,225,33);drag.setBounds(w-500,y+52,255,33);saveButton.setBounds(w-230,y+52,205,33);
 roll.setBounds(25,y+103,w-50,getHeight()-y-153);status.setBounds(25,getHeight()-40,w-50,25);
}
