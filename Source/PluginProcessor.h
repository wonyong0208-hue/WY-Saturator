#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <memory>
class WYAudioProcessor final : public juce::AudioProcessor {
public:
 WYAudioProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override {return true;}
 const juce::String getName() const override {return "WY Saturator";}
 bool acceptsMidi() const override {return false;}
 bool producesMidi() const override {return false;}
 bool isMidiEffect() const override {return false;}
 double getTailLengthSeconds() const override {return 0.0;}
 int getNumPrograms() override {return 1;}
 int getCurrentProgram() override {return 0;}
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override {return {};}
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 juce::AudioProcessorValueTreeState params;
 std::atomic<float> inputMeter{0.f},outputMeter{0.f};
 static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
private:
 juce::dsp::Oversampling<float> oversampling{2,2,juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,true,true};
 juce::AudioBuffer<float> dry;
 std::array<float,2> lowState{}, dcX{},dcY{};
 juce::SmoothedValue<float,juce::ValueSmoothingTypes::Linear> driveSmooth,outputSmooth,mixSmooth;
 int latency=0,delayIndex=0;
 std::array<std::vector<float>,2> delayLine;
 double fs=44100.0;
 static float saturate(float,int) noexcept;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WYAudioProcessor)
};
