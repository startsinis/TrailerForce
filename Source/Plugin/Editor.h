#pragma once
#include "Processor.h"
#include <functional>
class Form final : public juce::Component {
public:
 struct Item {std::unique_ptr<juce::Component> component;juce::Rectangle<int> bounds;};
 template<class T> T* add(std::unique_ptr<T> p,juce::Rectangle<int> b) {auto* raw=p.get();addAndMakeVisible(raw);items.push_back({std::move(p),b});return raw;}
 void resized() override {for(auto& i:items){auto b=i.bounds;i.component->setBounds(int(b.getX()*getWidth()/1060.),b.getY(),int(b.getWidth()*getWidth()/1060.),b.getHeight());}}
private:std::vector<Item> items;
};
class Roll final : public juce::Component {
public:
 tf::Sequence sequence;int lane=-1;double beat=0;
 void paint(juce::Graphics&) override;
};
class MidiDrag final : public juce::TextButton {
public:
 MidiDrag():TextButton("DRAG MIDI TO DAW"){}
 std::function<void()> beginDrag;
 void mouseDown(const juce::MouseEvent& e) override {juce::TextButton::mouseDown(e);started=false;}
 void mouseDrag(const juce::MouseEvent& e) override {juce::TextButton::mouseDrag(e);if(!started && e.getDistanceFromDragStart()>5){started=true;if(beginDrag)beginDrag();}}
private:bool started=false;
};
class TrailerForceEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit TrailerForceEditor(TrailerForceProcessor&);
 ~TrailerForceEditor() override;
 void paint(juce::Graphics&) override;void resized() override;
private:
 TrailerForceProcessor& p;tf::Settings working;bool refreshing=false,dirty=false;uint64_t seenRevision=0;double lastEdit=0;
 juce::LookAndFeel_V4 look;
 juce::TabbedComponent tabs{juce::TabbedButtonBar::TabsAtTop};
 std::array<Form*,8> forms{};
 juce::TextEditor *brief=nullptr,*interpretation=nullptr,*guide=nullptr,*production=nullptr;
 juce::Label status,transport;
 juce::TextButton generateButton{"GENERATE MIDI"},variationButton{"NEW VARIATION"},auditionButton{"AUDITION"},stopButton{"STOP / PANIC"},saveButton{"SAVE MIDI"};
 juce::ToggleButton monitor{"Sketch sound"},follow{"DAW play"},solo{"Solo selected lane"};
 juce::Slider gain;
 juce::ComboBox lane;
 MidiDrag drag;
 Roll roll;
 std::vector<std::function<void()>> reload;
 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> monitorAttachment,followAttachment;
 std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;
 std::unique_ptr<juce::FileChooser> chooser;
 juce::File dragFile;
 void timerCallback() override;void changed();void commit(bool gesture=false);void sync();void exportMidi(bool dragging);
 void label(int page,juce::String text,juce::Rectangle<int> bounds);
 void text(int page,juce::String value,juce::Rectangle<int> bounds);
 void slider(int page,juce::String name,juce::Rectangle<int> b,double min,double max,double step,std::function<double()> get,std::function<void(double)> set);
 void combo(int page,juce::String name,juce::Rectangle<int> b,juce::StringArray items,std::function<int()> get,std::function<void(int)> set);
 void toggle(int page,juce::String name,juce::Rectangle<int> b,std::function<bool()> get,std::function<void(bool)> set);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrailerForceEditor)
};
