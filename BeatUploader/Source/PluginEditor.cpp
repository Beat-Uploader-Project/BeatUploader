#include "PluginProcessor.h"
#include "PluginEditor.h"

BeatUploaderAudioProcessorEditor::BeatUploaderAudioProcessorEditor (BeatUploaderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

BeatUploaderAudioProcessorEditor::~BeatUploaderAudioProcessorEditor()
{
}

void BeatUploaderAudioProcessorEditor::paint (juce::Graphics& g)
{

}

void BeatUploaderAudioProcessorEditor::resized()
{

}
