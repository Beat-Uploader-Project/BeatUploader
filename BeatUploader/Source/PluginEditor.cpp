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

    // debugger (use it locally, but comment it out when you submit to remote)

    /*auto logFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
        .getChildFile("beat-uploader-log.txt");

    logger = std::make_unique<juce::FileLogger>(logFile, "BeatUploader started");
    juce::Logger::setCurrentLogger(logger.get());

    juce::Logger::writeToLog("Logger initialized");*/


    // colours map that assigns RGB colour to descriptive name
    colours["bg"] = juce::Colour(15, 15, 15);
    colours["labelBg"] = juce::Colour(31, 31, 31);

    colours["font"] = juce::Colour(223, 223, 223);
    colours["info"] = juce::Colour(209, 209, 209);
    colours["idle"] = juce::Colour(201, 201, 201);
    colours["success"] = juce::Colour(71, 242, 56);

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
    dscBox.setMultiLine(true);
    dscBox.setReturnKeyStartsNewLine(true);
    dscBox.setScrollbarsShown(true);

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

                    if (!audio.loadFileAsData(audioData)) {
                        output.setColour(juce::Label::textColourId, colours["error"]);
                        output.setText("Couldn't load audio file properly", juce::dontSendNotification);
                    }
                    else {
                        output.setColour(juce::Label::textColourId, colours["font"]);
                        output.setText("Audio uploaded", juce::dontSendNotification);

                        audioExt = audio.getFileExtension();
                        
                        juce::AudioFormatManager formatManager;
                        formatManager.registerBasicFormats();

                        if (auto* reader = formatManager.createReaderFor(audio)) {
                            duration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
                            audioChosen = true;

                            delete reader;
                        }
                        else {
                            output.setColour(juce::Label::textColourId, colours["error"]);
                            output.setText("Audio could not be read properly", juce::dontSendNotification);
                        }
                    }
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

                    if (!image.loadFileAsData(imageData)) {
                        output.setColour(juce::Label::textColourId, colours["error"]);
                        output.setText("Couldn't load iamge file properly", juce::dontSendNotification);
                    }
                    else {
                        output.setColour(juce::Label::textColourId, colours["font"]);
                        output.setText("Image uploaded", juce::dontSendNotification);

                        imageChosen = true;
                        imageExt = image.getFileExtension();
                    }
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

// checks for text file on user's PC
void BeatUploaderAudioProcessorEditor::checkForRefreshToken()
{
    std::string appdata; // get appdata folder path
    const char* env = std::getenv("APPDATA");

    if (env)
        appdata = env;
    else
        appdata = "notFound";

    std::string filePath = appdata + "\\BeatUploader\\refresh_token.txt";
    std::ifstream file(filePath); // open file, that was created earlier (and if not, continue with login function)

    if (file) {
        std::string code, email;
        try { // get access token and email stored in the file
            std::getline(file, code);
            std::getline(file, email);
        }
        catch (std::exception& e) { // if data cannot be read
            code = "";
            email = "";
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

// Google OAuth login process
void BeatUploaderAudioProcessorEditor::login()
{
    if (loggedIn) { // check if user is already logged in
        output.setColour(juce::Label::textColourId, colours["font"]);
        output.setText("Already logged in, click 'Change Account' button to change account", juce::dontSendNotification);
        return;
    }

    this->checkForRefreshToken(); // check for refresh_token.txt file in APPDATA to skip logging process
    if (accessTokenObtained) return;

    // below code creates listening socket and launches google oauth login page in the browser

    if (inProcess) // this stops user from creating another listening socket when one is already listening
        oauthReceiver->stopThread(1000); // stops currently running socket and reopnes it

    oauthReceiver = std::make_unique<OAuthReceiver>(8080); // initialize listener on localhost::8080

    oauthReceiver->setCallback([this](const juce::String& code) // assign callback, whan will happen when google authentication code is reveived
        {
            juce::MessageManager::callAsync([this, code]()
                {
                    googleAuthCode = code; // assign code

                    // inform user
                    output.setColour(juce::Label::textColourId, colours["font"]);
                    output.setText("Successfully logged in", juce::dontSendNotification);

                    loggedIn = true;
                    inProcess = false;
                });
        });

    oauthReceiver->startThread(); // start socket
    inProcess = true;

    juce::URL(loginURL).launchInDefaultBrowser(); // open google oauth login page
}

// creates text file on user's PC
void BeatUploaderAudioProcessorEditor::createRefreshToken(std::string email, std::string code)
{
    const char* appdata = std::getenv("APPDATA"); // find APPDATA folder
    if (!appdata) return;

    std::string filePath = std::string(appdata) + "\\BeatUploader";
    std::system(("mkdir \"" + filePath + "\" >nul 2>&1").c_str()); // create project folder in appdata

    filePath += "\\refresh_token.txt"; // create file
    std::ofstream file(filePath);

    if (file) {
        file << code << '\n' << email; // add data to file
    }
    file.close();
}

// check user input and evoke function to send data to API
void BeatUploaderAudioProcessorEditor::upload()
{
    // anti-spamming protection
    if (uploadedSuccessfully) {
        output.setColour(juce::Label::textColourId, colours["font"]);
        output.setText("You must delete the plugin and reopen it to upload another beat", juce::dontSendNotification);
        return;
    }

    // check user input
    if (!loggedIn) {
        output.setColour(juce::Label::textColourId, colours["error"]);
        output.setText("You must be logged in to continue", juce::dontSendNotification);
        return;
    }

    if (titleBox.getText() == "") {
        output.setColour(juce::Label::textColourId, colours["error"]);
        output.setText("Title was not provided", juce::dontSendNotification);
        return;
    }

    if (!audioChosen || !imageChosen) {
        output.setColour(juce::Label::textColourId, colours["error"]);
        output.setText("Audio or image was not provided", juce::dontSendNotification);
        return;
    }

    if (dscBox.getText() == "") {
        bool result = juce::AlertWindow::showOkCancelBox(
            juce::AlertWindow::WarningIcon,
            "Warning!",
            "Description was not provided, do you wish to continue?",
            "Yes",
            "Cancel",
            nullptr,
            nullptr
        );

        if (!result) return;
    }

    // send HTTP requests with data and access tokens or auth codes
    if (accessTokenObtained) this->sendData(); // accessToken variable contains a value
    else { // accessToken variable is empty
        this->getAccessToken();
        if (accessTokenObtained) this->sendData();
    }
}

// sends user input to /upload API endpoint
void BeatUploaderAudioProcessorEditor::sendData()
{
    juce::String base64EncodedAudio = audioData.toBase64Encoding(); // encode binary files to base64
    juce::String base64EncodedImage = imageData.toBase64Encoding();

    juce::DynamicObject* jsonObject = new juce::DynamicObject();
    jsonObject->setProperty("title", titleBox.getText());
    jsonObject->setProperty("dsc", dscBox.getText());
    jsonObject->setProperty("audio", base64EncodedAudio);
    jsonObject->setProperty("image", base64EncodedImage);
    jsonObject->setProperty("q", API_KEY);
    jsonObject->setProperty("access_token", accessToken);
    jsonObject->setProperty("audioExt", audioExt);
    jsonObject->setProperty("imageExt", imageExt);
    jsonObject->setProperty("duration", duration);

    juce::var jsonVar(jsonObject);
    juce::String jsonString = juce::JSON::toString(jsonVar); // create json

    juce::String headers = "Content-Type: application/json\r\nConnection: close\r\n";

    juce::URL url(API_URL + "/upload"); // internal api endpoint for uploading data
    url = url.withPOSTData(jsonString); // assign json body

    std::unique_ptr<juce::InputStream> stream(url.createInputStream( // conntect to api
        true,
        nullptr,
        nullptr,
        headers,
        10000,
        nullptr,
        nullptr,
        5
    ));

    if (stream != nullptr) { // this works similar to `getAccessToken` function, so go check it out
        juce::String response = stream->readEntireStreamAsString(); // read response
        juce::var parse = juce::JSON::parse(response); // response is ENTIRELY in JSON

        if (parse.isObject()) {
            juce::var result = parse["result"];

            if (result.isVoid()) {
                output.setColour(juce::Label::textColourId, colours["error"]);
                output.setText("Server error occured, please try again", juce::dontSendNotification);
            }

            std::string resultStr = std::string(result.toString().toRawUTF8());

            if (resultStr == "Success") {
                output.setColour(juce::Label::textColourId, colours["success"]);
                output.setText("Beat uploaded", juce::dontSendNotification);
                uploadedSuccessfully = true;
            }
            else if (resultStr == "Video") {
                output.setColour(juce::Label::textColourId, colours["error"]);
                output.setText("Video could not be made", juce::dontSendNotification);
            }
            else {
                output.setColour(juce::Label::textColourId, colours["error"]);
                output.setText("Video could not be uploaded to YouTube :(", juce::dontSendNotification);
            }
        }
        else {
            output.setColour(juce::Label::textColourId, colours["error"]);
            output.setText("Server error, please try again later", juce::dontSendNotification);
        }
    }
    else {
        output.setColour(juce::Label::textColourId, colours["error"]);
        output.setText("Unable to connect to Google account", juce::dontSendNotification);
    }

    stream.reset();
}

// send googleAuthCode to /getAccessToken API endpoint, that exchanges it to access token, refresh token and user email
void BeatUploaderAudioProcessorEditor::getAccessToken()
{
    // create JSON request to obtain access token to youtube api
    juce::DynamicObject* jsonObject = new juce::DynamicObject();
    jsonObject->setProperty("code", googleAuthCode);
    jsonObject->setProperty("q", API_KEY);

    juce::var jsonVar(jsonObject);
    juce::String jsonString = juce::JSON::toString(jsonVar);

    juce::String headers = "Content-Type: application/json\r\nConnection: close\r\n";// add headers

    juce::URL url(API_URL + "/getAccessToken"); // internal api endpoint, 
    url = url.withPOSTData(jsonString); // add json as request's body

    std::unique_ptr<juce::InputStream> stream(url.createInputStream( // connect to internal api
        true,
        nullptr,
        nullptr,
        headers,
        10000,
        nullptr,
        nullptr,
        5
    ));

    if (stream != nullptr) {
        juce::String response = stream->readEntireStreamAsString(); // receive response
        juce::var parsed = juce::JSON::parse(response); // parse it to json (response is ENTIRELY in JSON)

        if (parsed.isObject()) {
            // read values from json (this returns void if no value under that key is found)
            juce::var accessTokenVar = parsed["access_token"];
            juce::var refreshTokenVar = parsed["refresh_token"];
            juce::var emailVar = parsed["email"];

            if (accessTokenVar.isVoid() || refreshTokenVar.isVoid() || emailVar.isVoid()) { // check if they are void
                output.setColour(juce::Label::textColourId, colours["error"]); // this means Google didn't send access tokens
                output.setText("Google unabled the access :(", juce::dontSendNotification);
                return;
            }

            accessToken = accessTokenVar.toString(); // assign access token
            accessTokenObtained = true;

            std::string code = std::string(refreshTokenVar.toString().toRawUTF8());
            std::string email = std::string(emailVar.toString().toRawUTF8());

            this->createRefreshToken(email, code); // create refresh token in appdata/ for easier login process later
        }
        else { // API response is not a valid json
            output.setColour(juce::Label::textColourId, colours["error"]);
            output.setText("Server error, please try again later", juce::dontSendNotification);
        }
    }
    else { // this means that no connection with API was made
        output.setColour(juce::Label::textColourId, colours["error"]);
        output.setText("Unable to connect to the server", juce::dontSendNotification);
    }

    stream.reset();
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

    // below, elements are placed from one being the highest to one being the lowest
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
