#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include "env.h"

using AuthCodeCallback = std::function<void(const juce::String& code)>; // type of callback function

class OAuthReceiver : public juce::Thread
{
public:
    OAuthReceiver(int portToUse = 8080) // running listening server on port 8080 on another thread
        : juce::Thread("OAuthReceiverThread"), port(portToUse), shouldStop(false) {}

    void setCallback(AuthCodeCallback func) // sets function (callback) to run when authentication code is parsed
    {
        callback = std::move(func);
    }

    void run() override
    {
        juce::StreamingSocket serverSocket; // set up listening socket
        if (!serverSocket.createListener(port)) { // if unable to create socket on this port (8080)
            juce::URL(API_URL + "/static?status=error&reason=port").launchInDefaultBrowser(); // redirect to informative page
            return;
        }

        while (!threadShouldExit() && !shouldStop) { // wait for code to arrive while user is logging
            std::unique_ptr<juce::StreamingSocket> client(serverSocket.waitForNextConnection());

            if (client != nullptr) { // if user logged in and login page finally redirected to one that has authentication code
                char buffer[4096] = { 0 }; // creates buffer
                int bytesRead = client->read(buffer, sizeof(buffer), false); // read response

                if (bytesRead > 0) {
                    juce::String response(buffer, bytesRead); // get response

                    if (response.startsWith("GET ")) {
                        auto firstLine = response.upToFirstOccurrenceOf("\r\n", false, false); // authentication code is within the firstline of response
                        auto queryStart = firstLine.indexOf("?code="); // starting index of query parameters

                        if (queryStart != -1) {
                            auto codeStart = queryStart + 6;
                            auto codeEnd = firstLine.indexOfChar(codeStart, ' ');
                            juce::String code = firstLine.substring(codeStart, codeEnd).unquoted(); // parse code from query parameter

                            authCode = code;
                            shouldStop = true;

                            if (callback) { // run callback
                                juce::MessageManager::callAsync([cb = callback, code]() {
                                    cb(code);
                                });
                            }

                            juce::URL(API_URL + "/static?status=ok").launchInDefaultBrowser(); // redirect to success page
                        }
                        else {
                            juce::URL(API_URL + "/static?status=error&reason=parameters").launchInDefaultBrowser(); // redirect to error page
                            shouldStop = true;
                        }
                    }
                    else {
                        juce::URL(API_URL + "/static?status=error&reason=type").launchInDefaultBrowser();
                        shouldStop = true;
                    }
                }
                else {
                    juce::URL(API_URL + "/static?status=error&reason=empty").launchInDefaultBrowser();
                    shouldStop = true;
                }

                client->close();
            }

            juce::Thread::sleep(50); // sleep after every attempt to not burn CPU
        }

        serverSocket.close();
    }

private:
    int port;
    std::atomic<bool> shouldStop;
    juce::String authCode;
    AuthCodeCallback callback;
};
