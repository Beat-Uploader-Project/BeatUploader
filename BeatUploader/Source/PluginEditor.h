#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class BeatUploaderAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BeatUploaderAudioProcessorEditor (BeatUploaderAudioProcessor&);
    ~BeatUploaderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BeatUploaderAudioProcessor& audioProcessor;

    // google oauth login
    juce::TextButton login;

    // textbox input
    juce::TextEditor titleBox;
    juce::TextEditor dscBox;

    // file input and upload btn
    juce::TextButton audioSelect;
    juce::TextButton imageSelect;
    juce::TextButton upload;

    // files
    juce::File audio;
    juce::File image;

    // labels
    juce::Label output;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatUploaderAudioProcessorEditor)
};
