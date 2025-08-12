#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <map>
#include "env.h"
#include "OAuthReceiver.h"

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
    void sendData();
    void getAccessToken();
    void login();
    void checkForRefreshToken();
    void createRefreshToken(std::string email, std::string code);
    void changeAccount();

    //std::unique_ptr<juce::FileLogger> logger; // debugger

    // colours map (declared in constructor)
    std::map<juce::String, juce::Colour> colours;

    // constants
    int screenWidth = 400;
    int screenHeight = 650;

    // flags
    bool audioChosen = false;
    bool imageChosen = false;
    
    bool uploadedSuccessfully = false; // control to not auto spam, user must shut down the plugin to upload another beat after submitting one successfully
    bool loggedIn = false;
    bool accessTokenObtained = false;

    bool inProcess = false; // if listening socket is running, this will be set to true

    // google oauth login
    juce::TextButton loginBtn;
    juce::TextButton changeAccountBtn;

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
    juce::MemoryBlock audioData; // base64 encoding, necessary to send data through json
    juce::MemoryBlock imageData;
    juce::String audioExt; // e.g. .wav, .mp3
    juce::String imageExt; // .jpg, .gif
    double duration = 0.0;

    // labels
    juce::Label title;
    juce::Label loginInfo;
    juce::Label output;

    // fonts
    juce::Font titleFont, infoFont, boxFont, buttonFont;
    juce::Typeface::Ptr titleTypeface, infoTypeface, boxTypeface, buttonTypeface;

    std::unique_ptr<customTextButtonFont> lookAndFeel;

    // Google OAuth
    juce::String loginURL =
        "https://accounts.google.com/o/oauth2/v2/auth"
        "?client_id=1021325830189-2hdeafptk3lvarkhc8h1qh90if499jvl.apps.googleusercontent.com"
        "&redirect_uri=http://localhost:8080"
        "&response_type=code"
        "&scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fyoutube.upload%20openid%20email"
        "&access_type=offline"
        "&prompt=consent";

    juce::String googleAuthCode;
    juce::String accessToken;
    std::unique_ptr<OAuthReceiver> oauthReceiver;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatUploaderAudioProcessorEditor)
};
