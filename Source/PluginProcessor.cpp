#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
WYAudioProcessor::WYAudioProcessor():AudioProcessor(BusesProperties().withInput("입력",juce::AudioChannelSet::stereo(),true).withOutput("출력",juce::AudioChannelSet::stereo(),true)),params(*this,nullptr,"PARAMETERS",makeParameters()){}
juce::AudioProcessorValueTreeState::ParameterLayout WYAudioProcessor::makeParameters(){
 juce::AudioProcessorValueTreeState::ParameterLayout p;
 p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"drive",1},"드라이브",juce::NormalisableRange<float>(0,36,.1f),12));
 p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"mode",1},"질감",juce::StringArray{"부드럽게","강하게","진공관","테이프"},0));
 p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"tone",1},"톤",juce::NormalisableRange<float>(700,20000,1,.36f),14000));
 p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"mix",1},"믹스",juce::NormalisableRange<float>(0,100,.1f),100));
 p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"output",1},"출력",juce::NormalisableRange<float>(-24,12,.1f),-6));
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"autogain",1},"자동 레벨",true));
 p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"바이패스",false));
 return p;
}
bool WYAudioProcessor::isBusesLayoutSupported(const BusesLayout& b) const {auto o=b.getMainOutputChannelSet();return (o==juce::AudioChannelSet::mono()||o==juce::AudioChannelSet::stereo())&&b.getMainInputChannelSet()==o;}
void WYAudioProcessor::prepareToPlay(double sampleRate,int block){
 fs=sampleRate;
 oversampling.reset();oversampling.initProcessing(static_cast<size_t>(juce::jmax(1,block)));
 latency=static_cast<int>(std::round(oversampling.getLatencyInSamples()));setLatencySamples(latency);
 dry.setSize(2,juce::jmax(1,block));
 for(auto& v:delayLine)v.assign(static_cast<size_t>(latency+juce::jmax(1,block)+8),0.f);
 delayIndex=0;lowState.fill(0);dcX.fill(0);dcY.fill(0);
 for(auto* s:{&driveSmooth,&outputSmooth,&mixSmooth})s->reset(fs,.025);
 driveSmooth.setCurrentAndTargetValue(1);outputSmooth.setCurrentAndTargetValue(1);mixSmooth.setCurrentAndTargetValue(1);
}
float WYAudioProcessor::saturate(float x,int mode) noexcept {
 switch(mode){case 0:return std::tanh(x);case 1:return juce::jlimit(-1.f,1.f,x);case 2:return std::tanh(x+.18f*x*x)-.0f;default:return std::atan(1.7f*x)*(.63661977236f);}
}
void WYAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer&){
 juce::ScopedNoDenormals guard;
 const int n=buffer.getNumSamples(),ch=juce::jmin(2,buffer.getNumChannels());
 for(int c=ch;c<buffer.getNumChannels();++c)buffer.clear(c,0,n);
 if(n==0||ch==0)return;
 const float driveDb=params.getRawParameterValue("drive")->load();
 const float outDb=params.getRawParameterValue("output")->load();
 const float wet=params.getRawParameterValue("mix")->load()*.01f;
 const float tone=params.getRawParameterValue("tone")->load();
 const int mode=juce::jlimit(0,3,static_cast<int>(params.getRawParameterValue("mode")->load()));
 const bool autoGain=params.getRawParameterValue("autogain")->load()>.5f;
 const bool bypass=params.getRawParameterValue("bypass")->load()>.5f;
 driveSmooth.setTargetValue(juce::Decibels::decibelsToGain(driveDb));
 outputSmooth.setTargetValue(juce::Decibels::decibelsToGain(outDb));mixSmooth.setTargetValue(wet);
 float inPeak=0.f,outPeak=0.f;
 for(int c=0;c<ch;++c){dry.copyFrom(c,0,buffer,c,0,n);inPeak=juce::jmax(inPeak,buffer.getMagnitude(c,0,n));}
 juce::dsp::AudioBlock<float> b(buffer);
 auto up=oversampling.processSamplesUp(b);
 const float coeff=static_cast<float>(std::exp(-2.0*juce::MathConstants<double>::pi*juce::jmin(static_cast<double>(tone),fs*1.8)/(fs*4.0)));
 const float drive=juce::Decibels::decibelsToGain(driveDb);
 const float normalization=autoGain ? 1.f/std::sqrt(juce::jmax(1.f,drive)) : 1.f;
 for(int c=0;c<ch;++c){
  auto* ptr=up.getChannelPointer(static_cast<size_t>(c));float z=lowState[static_cast<size_t>(c)];
  for(size_t i=0;i<up.getNumSamples();++i){float y=saturate(ptr[i]*drive,mode)*normalization;z+=(1.f-coeff)*(y-z);ptr[i]=z;}
  lowState[static_cast<size_t>(c)]=z;
 }
 oversampling.processSamplesDown(b);
 const int length=static_cast<int>(delayLine[0].size());
 for(int i=0;i<n;++i){
  const float g=outputSmooth.getNextValue(),m=mixSmooth.getNextValue();driveSmooth.skip(1);
  for(int c=0;c<ch;++c){
   auto& line=delayLine[static_cast<size_t>(c)];
   const float input=dry.getSample(c,i);
   line[static_cast<size_t>(delayIndex)]=input;
   int read=delayIndex-latency;if(read<0)read+=length;
   const float alignedDry=line[static_cast<size_t>(read)];
   float y=bypass?alignedDry:g*(m*buffer.getSample(c,i)+(1.f-m)*alignedDry);
   // DC blocker after asymmetric nonlinear stages.
   const size_t idx=static_cast<size_t>(c);
   if(!bypass){const float dc=y-dcX[idx]+.9995f*dcY[idx];dcX[idx]=y;dcY[idx]=dc;y=dc;}
   buffer.setSample(c,i,y);outPeak=juce::jmax(outPeak,std::abs(y));
  }
  if(++delayIndex>=length)delayIndex=0;
 }
 inputMeter.store(inPeak);outputMeter.store(outPeak);
}
juce::AudioProcessorEditor* WYAudioProcessor::createEditor(){return new WYEditor(*this);}
void WYAudioProcessor::getStateInformation(juce::MemoryBlock& dest){if(auto xml=params.copyState().createXml())copyXmlToBinary(*xml,dest);}
void WYAudioProcessor::setStateInformation(const void* data,int size){if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(params.state.getType()))params.replaceState(juce::ValueTree::fromXml(*xml));}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new WYAudioProcessor();}
