#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OAuthReceiver.h"

#include <fstream>
#include <cstdlib>
#include <string>

BeatUploaderAudioProcessorEditor::BeatUploaderAudioProcessorEditor (BeatUploaderAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (screenWidth, screenHeight);

    // debugger

    //auto logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
    //    .getChildFile("beat-uploader-log.txt");

    //logger = std::make_unique<juce::FileLogger>(logFile, "BeatUploader started");
    //juce::Logger::setCurrentLogger(logger.get());

    //juce::Logger::writeToLog("Logger initialized");


    // colours map that assigns RGB colour to descriptive name
    colours["bg"] = juce::Colour(15, 15, 15);
    colours["labelBg"] = juce::Colour(31, 31, 31);

    colours["font"] = juce::Colour(223, 223, 223);
    colours["info"] = juce::Colour(209, 209, 209);
    colours["idle"] = juce::Colour(201, 201, 201);

    colours["btnOnTxt"] = juce::Colour(217, 217, 217);
    colours["btnOnBg"] = juce::Colour(26, 26, 26);

    colours["hl"] = juce::Colour(92, 176, 255);
    colours["hlTxt"] = juce::Colour(245, 245, 245);

    colours["error"] = juce::Colour(230, 32, 32);

    // fonts setup
    titleTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MichromaRegular_ttf,
        BinaryData::MichromaRegular_ttfSize
    );
    titleFont = juce::Font(titleTypeface).withHeight(42.0f);

    infoTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MontserratLightItalic_ttf,
        BinaryData::MontserratLightItalic_ttfSize
    );
    infoFont = juce::Font(infoTypeface).withHeight(19.0f);

    boxTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MontserratSemiBold_ttf,
        BinaryData::MontserratSemiBold_ttfSize
    );
    boxFont = juce::Font(boxTypeface).withHeight(23.0f);

    buttonTypeface = juce::Typeface::createSystemTypefaceFor(
        BinaryData::MontserratRegular_ttf,
        BinaryData::MontserratRegular_ttfSize
    );
    buttonFont = juce::Font(buttonTypeface).withHeight(20.0f);


    // makes visible and adds functionality to:
    // logging button
    addAndMakeVisible(loginBtn);
    loginBtn.onClick = [this]() {
        this->login();
    };

    // textboxes and labels
    addAndMakeVisible(titleBox);
    addAndMakeVisible(dscBox);

    addAndMakeVisible(title);
    addAndMakeVisible(info);
    addAndMakeVisible(output);

    // audio and image input
    addAndMakeVisible(audioSelect);
    audioSelect.onClick = [this]()
    {
        output.setText("", juce::dontSendNotification);

        auto chooser = std::make_shared<juce::FileChooser>(
            "Select an audio file",
            juce::File{},
            "*.mp3;*.wav;*.flac;*.ogg"
        );

        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto result = fc.getResult();

                if (!result.existsAsFile()) {
                    output.setColour(juce::Label::textColourId, colours["error"]);
                    output.setText("Audio was not chosen", juce::dontSendNotification);
                }
                else {
                    audio = result;
                    audioChosen = true;
                }
            });
    };

    addAndMakeVisible(imageSelect);
    imageSelect.onClick = [this]()
    {
        output.setText("", juce::dontSendNotification);

        auto chooser = std::make_shared<juce::FileChooser>(
            "Select image file",
            juce::File{},
            "*.jpg;*.png;*.gif"
        );

        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto result = fc.getResult();

                if (!result.existsAsFile()) {
                    output.setColour(juce::Label::textColourId, colours["error"]);
                    output.setText("Image was not chosen", juce::dontSendNotification);
                }
                else {
                    image = result;
                    imageChosen = true;
                }
            });
    };

    // upload button
    addAndMakeVisible(uploadBtn);
    uploadBtn.onClick = [this]() {
        this->upload();
    };


    // set up font for TextButtons
    lookAndFeel = std::make_unique<customTextButtonFont>(buttonFont);

    loginBtn.setLookAndFeel(lookAndFeel.get());
    audioSelect.setLookAndFeel(lookAndFeel.get());
    imageSelect.setLookAndFeel(lookAndFeel.get());
    uploadBtn.setLookAndFeel(lookAndFeel.get());
}
BeatUploaderAudioProcessorEditor::~BeatUploaderAudioProcessorEditor(){}

void BeatUploaderAudioProcessorEditor::checkForRefreshToken()
{
    char* appdata = std::getenv("APPDATA");
    if (!appdata) {
        output.setColour(juce::Label::textColourId, colours["font"]);
        output.setText("Opening the browser...", juce::dontSendNotification);
        return;
    }

    std::string filePath = std::string(appdata) + "\\BeatUploader\\refresh_token.txt";
    std::ifstream file(filePath);

    if (file) {
        std::string code, email;
        try {
            std::getline(file, code);
            std::getline(file, email);
        }
        catch (std::exception& e) { // if data of the files cannot be read
            output.setColour(juce::Label::textColourId, colours["font"]);
            output.setText("Opening the browser...", juce::dontSendNotification);
            return;
        }

        if (code != "" && email != "") {
            accessToken = code;
            accessTokenObtained = loggedIn = true;

            output.setColour(juce::Label::textColourId, colours["font"]);
            output.setText("Successfully logged in as: " + email, juce::dontSendNotification);
        }
    }
    if (!accessTokenObtained) {
        output.setColour(juce::Label::textColourId, colours["font"]);
        output.setText("Opening the browser...", juce::dontSendNotification);
    }

    file.close();
}

void BeatUploaderAudioProcessorEditor::login()
{
    if (!loginStarted && !loggedIn) {
        if (!accessTokenObtained) {
            checkForRefreshToken();
            if (accessTokenObtained) return;
        }

        oauthReceiver = std::make_unique<OAuthReceiver>(8080);
        loginStarted = true;

        oauthReceiver->setCallback([this](const juce::String& code)
            {
                juce::MessageManager::callAsync([this, code]()
                    {
                        googleAuthCode = code; // assign code

                        // inform user
                        output.setColour(juce::Label::textColourId, colours["font"]);
                        output.setText("Successfully logged in", juce::dontSendNotification);

                        loggedIn = true;
                    });
            });

        oauthReceiver->startThread();

        juce::URL(loginURL).launchInDefaultBrowser();
    }
    else if (loginStarted && !loggedIn) {
        output.setColour(juce::Label::textColourId, colours["error"]);
        output.setText("Login process failed, delete the plugin and open it again", juce::dontSendNotification);
    }
    else {
        output.setColour(juce::Label::textColourId, colours["font"]);
        output.setText("To change account, delete the plugin and open it again", juce::dontSendNotification);
    }
}

void BeatUploaderAudioProcessorEditor::createRefreshToken(std::string email, std::string code)
{
    const char* appdata = std::getenv("APPDATA");
    if (!appdata) return;

    std::string filePath = std::string(appdata) + "\\BeatUploader";
    std::system(("mkdir \"" + filePath + "\" >nul 2>&1").c_str());

    filePath += "\\refresh_token.txt";
    std::ofstream file(filePath);

    if (file) {
        file << code << '\n' << email;
    }
    file.close();
}

void BeatUploaderAudioProcessorEditor::upload()
{

}

void BeatUploaderAudioProcessorEditor::paint (juce::Graphics& g)
{
    // sets colour, font and text for each gui element
    // font for TextButtons is set up in constructor
    g.fillAll(colours["bg"]);

    title.setColour(juce::Label::textColourId, colours["font"]);
    title.setColour(juce::Label::backgroundColourId, colours["bg"]);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(titleFont);
    title.setText("Beat Uploader", juce::dontSendNotification);

    loginBtn.setColour(juce::TextButton::textColourOffId, colours["font"]);
    loginBtn.setColour(juce::TextButton::textColourOnId, colours["btnOnTxt"]);
    loginBtn.setColour(juce::TextButton::buttonColourId, colours["labelBg"]);
    loginBtn.setColour(juce::TextButton::buttonOnColourId, colours["btnOnBg"]);
    loginBtn.setButtonText("Login");

    info.setColour(juce::Label::textColourId, colours["info"]);
    info.setColour(juce::Label::backgroundColourId, colours["bg"]);
    info.setFont(infoFont);
    info.setText("Google account (YouTube channel)", juce::dontSendNotification);

    titleBox.setColour(juce::TextEditor::textColourId, colours["font"]);
    titleBox.setColour(juce::TextEditor::backgroundColourId, colours["labelBg"]);
    titleBox.setColour(juce::TextEditor::highlightedTextColourId, colours["hlTxt"]);
    titleBox.setColour(juce::TextEditor::highlightColourId, colours["hl"]);
    titleBox.setFont(boxFont);
    titleBox.setJustification(juce::Justification::centredLeft);
    titleBox.setTextToShowWhenEmpty("Title...", colours["idle"]);

    dscBox.setColour(juce::TextEditor::textColourId, colours["font"]);
    dscBox.setColour(juce::TextEditor::backgroundColourId, colours["labelBg"]);
    dscBox.setColour(juce::TextEditor::highlightedTextColourId, colours["hlTxt"]);
    dscBox.setColour(juce::TextEditor::highlightColourId, colours["hl"]);
    dscBox.setFont(boxFont);
    dscBox.setTextToShowWhenEmpty("Description...", colours["idle"]);

    audioSelect.setColour(juce::TextButton::textColourOffId, colours["font"]);
    audioSelect.setColour(juce::TextButton::textColourOnId, colours["btnOnTxt"]);
    audioSelect.setColour(juce::TextButton::buttonColourId, colours["labelBg"]);
    audioSelect.setColour(juce::TextButton::buttonOnColourId, colours["btnOnBg"]);
    audioSelect.setButtonText("Audio");

    imageSelect.setColour(juce::TextButton::textColourOffId, colours["font"]);
    imageSelect.setColour(juce::TextButton::textColourOnId, colours["btnOnTxt"]);
    imageSelect.setColour(juce::TextButton::buttonColourId, colours["labelBg"]);
    imageSelect.setColour(juce::TextButton::buttonOnColourId, colours["btnOnBg"]);
    imageSelect.setButtonText("Image");

    uploadBtn.setColour(juce::TextButton::textColourOffId, colours["font"]);
    uploadBtn.setColour(juce::TextButton::textColourOnId, colours["btnOnTxt"]);
    uploadBtn.setColour(juce::TextButton::buttonColourId, colours["labelBg"]);
    uploadBtn.setColour(juce::TextButton::buttonOnColourId, colours["btnOnBg"]);
    uploadBtn.setButtonText("Upload");

    output.setColour(juce::Label::backgroundColourId, colours["bg"]);
    output.setJustificationType(juce::Justification::centred);
    output.setFont(boxFont);
}

void BeatUploaderAudioProcessorEditor::resized()
{
    // places gui elements on the screen
    int marginX = 16;
    int marginY = 10;
    int spacing = 18;
    int cursor = marginY; // points to current place on Y axis to place next label

    title.setBounds(marginX, cursor, screenWidth - (2 * marginX), 42);
    cursor += (42 + spacing);

    loginBtn.setBounds(marginX, cursor, 110, 20);
    info.setBounds(marginX + 110 + spacing, cursor, screenWidth - ((2 * marginX) + 110 + spacing), 19);
    cursor += (20 + (4 * spacing));

    titleBox.setBounds(marginX, cursor, 110, 23);
    cursor += (23 + spacing);

    dscBox.setBounds(marginX, cursor, screenWidth - (2 * marginX), 216);
    cursor += (216 + spacing);

    audioSelect.setBounds(marginX, cursor, 110, 20);
    imageSelect.setBounds(screenWidth - (marginX + 110), cursor, 110, 20);
    cursor += (20 + (2 * spacing));

    uploadBtn.setBounds(screenWidth - (marginX + 110), cursor, 110, 20);
    
    output.setBounds(marginX, screenHeight - (marginY + 23), screenWidth - (2 * marginX), 23);
}
