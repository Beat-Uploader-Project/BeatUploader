#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <map>

// custom font for text buttons
class customTextButtonFont : public juce::LookAndFeel_V4
{
public:
    customTextButtonFont(const juce::Font& f) : customFont(f) {}

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override
    {
        return customFont;
    }

private:
    juce::Font customFont;
};

class BeatUploaderAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    BeatUploaderAudioProcessorEditor (BeatUploaderAudioProcessor&);
    ~BeatUploaderAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    BeatUploaderAudioProcessor& audioProcessor;

    void upload();
    void login();

    // colours map (declared in constructor)
    std::map<juce::String, juce::Colour> colours;

    // constants
    int screenWidth = 400;
    int screenHeight = 650;
    juce::String API_URL = "not avaliable yet";

    // control variables
    bool audioChosen = false;
    bool imageChosen = false;
    bool uploadedSuccessfully = false; // control to not auto spam, user must shut down the plugin to upload another beat after submitting one successfully
    bool loggedIn = false;

    // google oauth login
    juce::TextButton loginBtn;

    // textbox input
    juce::TextEditor titleBox;
    juce::TextEditor dscBox;

    // file input and upload btn
    juce::TextButton audioSelect;
    juce::TextButton imageSelect;
    juce::TextButton uploadBtn;

    // files
    juce::File audio;
    juce::File image;

    // labels
    juce::Label title;
    juce::Label info;
    juce::Label output;

    // fonts
    juce::Font titleFont, infoFont, boxFont, buttonFont;
    juce::Typeface::Ptr titleTypeface, infoTypeface, boxTypeface, buttonTypeface;

    std::unique_ptr<customTextButtonFont> lookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatUploaderAudioProcessorEditor)
};
