#include "PluginEditor.h"
#include <cmath>
namespace {const juce::Colour bg{0xff10141b},panel{0xff1a222d},ink{0xffeef4f8},muted{0xff98a8b7},accent{0xff4de3b3};}
WYEditor::Dial::Dial(){setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,24);setColour(juce::Slider::rotarySliderFillColourId,accent);setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff354352));setColour(juce::Slider::thumbColourId,ink);setColour(juce::Slider::textBoxTextColourId,ink);setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);setColour(juce::Slider::textBoxBackgroundColourId,panel);}
WYEditor::WYEditor(WYAudioProcessor& p):AudioProcessorEditor(p),processor(p){
 setSize(800,490);
 title.setText("WY  /  SATURATOR",juce::dontSendNotification);title.setFont(juce::Font(juce::FontOptions(29.f,juce::Font::bold)));title.setColour(juce::Label::textColourId,ink);addAndMakeVisible(title);
 subtitle.setText("소리의 온도와 질감을 디자인하세요",juce::dontSendNotification);subtitle.setColour(juce::Label::textColourId,muted);addAndMakeVisible(subtitle);
 const char* ids[]={"drive","tone","mix","output"};const char* names[]={"드라이브","톤 (Hz)","원음 혼합 (%)","출력 (dB)"};
 for(size_t i=0;i<4;++i){addAndMakeVisible(dials[i]);labels[i].setText(juce::String::fromUTF8(names[i]),juce::dontSendNotification);labels[i].setJustificationType(juce::Justification::centred);labels[i].setColour(juce::Label::textColourId,ink);addAndMakeVisible(labels[i]);dialAttachments[i]=std::make_unique<SliderAttachment>(processor.params,ids[i],dials[i]);}
 modeLabel.setText("새츄레이션 질감",juce::dontSendNotification);modeLabel.setColour(juce::Label::textColourId,ink);addAndMakeVisible(modeLabel);
 for(int i=0;i<4;++i)mode.addItem(juce::StringArray{"부드럽게","강하게","진공관","테이프"}[i],i+1);
 mode.setColour(juce::ComboBox::backgroundColourId,panel);mode.setColour(juce::ComboBox::textColourId,ink);addAndMakeVisible(mode);modeAttachment=std::make_unique<ComboAttachment>(processor.params,"mode",mode);
 autoGain.setButtonText("자동 레벨 보정");bypass.setButtonText("바이패스");for(auto* b:{&autoGain,&bypass}){b->setColour(juce::ToggleButton::textColourId,ink);b->setColour(juce::ToggleButton::tickColourId,accent);addAndMakeVisible(*b);}
 gainAttachment=std::make_unique<ButtonAttachment>(processor.params,"autogain",autoGain);bypassAttachment=std::make_unique<ButtonAttachment>(processor.params,"bypass",bypass);
 meterLabel.setText("입력                                      출력",juce::dontSendNotification);meterLabel.setColour(juce::Label::textColourId,muted);addAndMakeVisible(meterLabel);
 startTimerHz(25);
}
void WYEditor::resized(){title.setBounds(34,23,440,42);subtitle.setBounds(36,67,440,25);
 for(int i=0;i<4;++i){const int x=32+i*191;labels[static_cast<size_t>(i)].setBounds(x,145,164,28);dials[static_cast<size_t>(i)].setBounds(x,173,164,145);}
 modeLabel.setBounds(40,340,160,25);mode.setBounds(40,369,210,36);autoGain.setBounds(294,369,180,35);bypass.setBounds(510,369,150,35);meterLabel.setBounds(535,45,235,20);
}
void WYEditor::timerCallback(){inMeter=juce::jmax(processor.inputMeter.load(),inMeter*.84f);outMeter=juce::jmax(processor.outputMeter.load(),outMeter*.84f);repaint(juce::Rectangle<int>(530,70,235,40));}
void WYEditor::paint(juce::Graphics& g){g.fillAll(bg);g.setColour(panel);g.fillRoundedRectangle(22.f,117.f,756.f,214.f,18.f);g.fillRoundedRectangle(22.f,337.f,756.f,92.f,14.f);
 g.setColour(juce::Colour(0xff283440));g.drawLine(32.f,108.f,767.f,108.f,1.f);
 const float levels[]={inMeter,outMeter};for(int i=0;i<2;++i){const float x=535.f+119.f*i;g.setColour(juce::Colour(0xff303e4c));g.fillRoundedRectangle(x,76.f,105.f,10.f,4.f);g.setColour(levels[i]>.99f?juce::Colour(0xffff6b6b):accent);g.fillRoundedRectangle(x,76.f,105.f*juce::jlimit(0.f,1.f,levels[i]),10.f,4.f);}
 g.setColour(muted);g.setFont(13.f);g.drawText("4× 오버샘플링 · 모노/스테레오 · DAW 자동화",35,450,730,22,juce::Justification::centred);
}
