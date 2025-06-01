#include <iostream>
#include <memory>
#include <thread>
#include "audio_passthrough.h"
#include "effect_chain.h"
#include "effects.h"

std::atomic<bool> running{true};

void userInterface(std::shared_ptr<EffectChain> chain)
{
    std::string input;
    chain->listEffects();

    while (running)
    {
        std::cout << "\n[COMMANDS]\n 1:Toggle Dist | 2:Toggle Chorus | 3:Toggle Delay\n g:Set Input Gain\n d:Dist Params | c:Chorus Params | l:Delay Params\n q:Quit\n> ";
        std::getline(std::cin, input);

        if (input == "1")
        {
            chain->toggleEffect(0);
        }
        else if (input == "2")
        {
            chain->toggleEffect(1);
        }
        else if (input == "3")
        {
            chain->toggleEffect(2);
        }
        else if (input == "g")
        {
            float g;
            std::cout << "Input Gain [0-10]: ";
            if (std::cin >> g)
            {
                if (g < 0.0f || g > 10.0f)
                    std::cout << "Value out of range!\n";
                else
                    chain->setInputGain(g);
            }
            else
            {
                std::cout << "Invalid input!\n";
                std::cin.clear();
                std::cin.ignore(10000, '\n');
            }
        }

        // ---- DISTORTION PARAMS ----
        else if (input == "d")
        {
            auto dist = std::dynamic_pointer_cast<DistortionEffect>(chain->getEffect(0));
            if (!dist)
                continue;

            std::cout << "[Distortion] Gain = " << dist->getGain()
                      << ", Mix = " << dist->getMix() << "\n";
            std::cout << "Change (1:Gain, 2:Mix, 0:Cancel): ";
            std::getline(std::cin, input);

            if (input == "1")
            {
                float val;
                std::cout << "New Gain [0-10]: ";
                if (std::cin >> val)
                {
                    if (val < 0.0f || val > 10.0f)
                        std::cout << "Value out of range!\n";
                    else
                        dist->setGain(val);
                }
                else
                {
                    std::cout << "Invalid input!\n";
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                }
            }
            else if (input == "2")
            {
                float val;
                std::cout << "New Mix [0 - 1]: ";
                if (std::cin >> val)
                {
                    if (val < 0.0f || val > 1.0f)
                        std::cout << "Value out of range!\n";
                    else
                        dist->setMix(val);
                }
                else
                {
                    std::cout << "Invalid input!\n";
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                }
            }
        }

        // ---- CHORUS PARAMS ----
        else if (input == "c")
        {
            auto chorus = std::dynamic_pointer_cast<ChorusEffect>(chain->getEffect(1));
            if (!chorus)
                continue;

            std::cout << "[Chorus] Rate = " << chorus->getRate()
                      << ", Depth = " << chorus->getDepth() << "\n";
            std::cout << "Change (1:Rate, 2:Depth, 0:Cancel): ";
            std::getline(std::cin, input);

            if (input == "1")
            {
                float val;
                std::cout << "New Rate [0.0 - 1.0]: ";
                if (std::cin >> val)
                {
                    if (val < 0.0f || val > 1.0f)
                        std::cout << "Value out of range!\n";
                    else
                        chorus->setRate(val);
                }
                else
                {
                    std::cout << "Invalid input!\n";
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                }
            }
            else if (input == "2")
            {
                float val;
                std::cout << "New Depth [0.0 - 0.01]: ";
                if (std::cin >> val)
                {

                    if (val < 0.0f || val > 0.01f)
                        std::cout << "Value out of range!\n";
                    else
                        chorus->setDepth(val);
                }
                else
                {
                    std::cout << "Invalid input!\n";
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                }
            }
        }

        // ---- DELAY PARAMS ----
        else if (input == "l")
        {
            auto delay = std::dynamic_pointer_cast<DelayEffect>(chain->getEffect(2));
            if (!delay)
                continue;

            std::cout << "[Delay] Delay Time = " << delay->getDelayTime()
                      << " sec\nChange (1:Time, 0:Cancel): ";
            std::getline(std::cin, input);

            if (input == "1")
            {
                float val;
                std::cout << "New Delay Time [0.05 - 1.0]: ";
                if (std::cin >> val)
                {

                    if (val < 0.05f || val > 1.0f)
                        std::cout << "Value out of range!\n";
                    else
                        delay->setDelayTime(val);
                }
                else
                {
                    std::cout << "Invalid input!\n";
                    std::cin.clear();
                    std::cin.ignore(10000, '\n');
                }
            }
        }

        else if (input == "q")
        {
            running = false;
            break;
        }

        chain->listEffects();
    }
}
int main()
{
    try
    {
        unsigned int sampleRate = 48000;
        auto chain = std::make_shared<EffectChain>();
        chain->setInputGain(8.0f);

        auto distortion = std::make_shared<DistortionEffect>(8.0f, 1.0f);
        auto chorus = std::make_shared<ChorusEffect>(sampleRate);
        auto delay = std::make_shared<DelayEffect>(sampleRate);

        chain->addEffect(distortion, "Distortion", false);
        chain->addEffect(chorus, "Chorus", false);
        chain->addEffect(delay, "Delay", false);

        std::thread uiThread(userInterface, chain);

        AudioPassthrough passthrough(chain.get());
        passthrough.start(sampleRate);

        if (uiThread.joinable())
            uiThread.join();
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}