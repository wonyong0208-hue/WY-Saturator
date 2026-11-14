#pragma once
#include "PluginProcessor.h"
class WYEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit WYEditor(WYAudioProcessor&);
 void paint(juce::Graphics&) override;
 void resized() override;
private:
 WYAudioProcessor& processor;
 struct Dial:juce::Slider {Dial();};
 std::array<Dial,4> dials;
 std::array<juce::Label,4> labels;
 juce::ComboBox mode;
 juce::ToggleButton autoGain,bypass;
 juce::Label title,subtitle,modeLabel,meterLabel;
 using SliderAttachment=juce::AudioProcessorValueTreeState::SliderAttachment;
 using ComboAttachment=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 using ButtonAttachment=juce::AudioProcessorValueTreeState::ButtonAttachment;
 std::array<std::unique_ptr<SliderAttachment>,4> dialAttachments;
 std::unique_ptr<ComboAttachment> modeAttachment;
 std::unique_ptr<ButtonAttachment> gainAttachment,bypassAttachment;
 float inMeter=0.f,outMeter=0.f;
 void timerCallback() override;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WYEditor)
};
