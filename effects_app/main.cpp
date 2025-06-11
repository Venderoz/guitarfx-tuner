#include "audio_passthrough.h"
#include "effect_chain.h"

std::atomic<bool> running{true};

void userInterface(std::shared_ptr<EffectChain> chain)
{
    std::string input;
    chain->listEffects();

    while (running)
    {
        std::cout << "\n[COMMANDS]\n";
        std::cout << " \033[33mG\033[0m: Set Input Gain\n";
        std::cout << " \033[33m1\033[0m: Toggle Dist | \033[33m2\033[0m: Toggle Chorus | \033[33m3\033[0m: Toggle Delay\n";
        std::cout << " \033[33mD\033[0m: Dist Params | \033[33mC\033[0m: Chorus Params | \033[33mL\033[0m: Delay Params\n";
        std::cout << " \033[33mQ\033[0m: Quit\n> ";
        std::getline(std::cin, input);

        for (auto &c : input)
            c = std::toupper(c);

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
        else if (input == "G")
        {
            std::cout << "Input Gain [0-10]: ";
            std::string valStr;
            std::getline(std::cin, valStr);
            std::stringstream ss(valStr);
            float val;
            if (ss >> val && val >= 0.0f && val <= 10.0f)
                chain->setInputGain(val);
            else
                std::cout << "\033[1;31mInvalid or out of range!\033[0m\n";
        }

        // Distortion
        else if (input == "D")
        {
            auto dist = std::dynamic_pointer_cast<DistortionEffect>(chain->getEffect(0));
            if (!dist)
                continue;

            std::cout << "[Distortion] Gain = " << dist->getGain()
                      << ", Mix = " << dist->getMix() << "\n";
            std::cout << "Change (\033[33m1\033[0m: Gain, \033[33m2\033[0m: Mix, \033[33m0\033[0m: Cancel): ";
            std::getline(std::cin, input);

            if (input == "1")
            {
                std::cout << "New Gain [0-10]: ";
                std::string valStr;
                std::getline(std::cin, valStr);
                std::stringstream ss(valStr);
                float val;
                if (ss >> val && val >= 0.0f && val <= 10.0f)
                    dist->setGain(val);
                else
                    std::cout << "\033[1;31mInvalid or out of range!\033[0m\n";
            }
            else if (input == "2")
            {
                std::cout << "New Mix [0-1]: ";
                std::string valStr;
                std::getline(std::cin, valStr);
                std::stringstream ss(valStr);
                float val;
                if (ss >> val && val >= 0.0f && val <= 1.0f)
                    dist->setMix(val);
                else
                    std::cout << "\033[1;31mInvalid or out of range!\033[0m\n";
            }
            else
            {
                if (input != "0")
                {
                    std::cout << "\033[1;31mInvalid input!\033[0m\n";
                }
            }
        }

        // Chorus
        else if (input == "C")
        {
            auto chorus = std::dynamic_pointer_cast<ChorusEffect>(chain->getEffect(1));
            if (!chorus)
                continue;

            std::cout << "[Chorus] Rate = " << chorus->getRate()
                      << ", Depth = " << chorus->getDepth() * 100 << "\n";
            std::cout << "Change (\033[33m1\033[0m: Rate, \033[33m2\033[0m: Depth, \033[33m0\033[0m: Cancel): ";
            std::getline(std::cin, input);

            if (input == "1")
            {
                std::cout << "New Rate [0.0 - 1.0]: ";
                std::string valStr;
                std::getline(std::cin, valStr);
                std::stringstream ss(valStr);
                float val;
                if (ss >> val && val >= 0.0f && val <= 1.0f)
                    chorus->setRate(val);
                else
                    std::cout << "\033[1;31mInvalid or out of range!\033[0m\n";
            }
            else if (input == "2")
            {
                std::cout << "New Depth [0.0 - 1.0]: ";
                std::string valStr;
                std::getline(std::cin, valStr);
                std::stringstream ss(valStr);
                float val;
                if (ss >> val && val >= 0.0f && val <= 1.0f)
                    chorus->setDepth(val);
                else
                    std::cout << "\033[1;31mInvalid or out of range!\033[0m\n";
            }
            else
            {
                if (input != "0")
                {
                    std::cout << "\033[1;31mInvalid input!\033[0m\n";
                }
            }
        }

        // Delay
        else if (input == "L")
        {
            auto delay = std::dynamic_pointer_cast<DelayEffect>(chain->getEffect(2));
            if (!delay)
                continue;

            std::cout << "[Delay] Delay Time = " << delay->getDelayTime()
                      << " sec\nChange (\033[33m1\033[0m: Time, \033[33m0\033[0m: Cancel): ";
            std::getline(std::cin, input);

            if (input == "1")
            {
                std::cout << "New Delay Time [0.0 - 1.0]: ";
                std::string valStr;
                std::getline(std::cin, valStr);
                std::stringstream ss(valStr);
                float val;
                if (ss >> val && val >= 0.0f && val <= 1.0f)
                    delay->setDelayTime(val);
                else
                    std::cout << "\033[1;31mInvalid or out of range!\033[0m\n";
            }
            else
            {
                if (input != "0")
                {
                    std::cout << "\033[1;31mInvalid input!\033[0m\n";
                }
            }
        }
        else if (input == "Q")
        {
            running = false;
            break;
        }
        else
        {
            std::cout << "\033[1;31mInvalid input!\033[0m\n";
        }

        std::cout << "\n--------------------------------------------------\n";
        chain->listEffects();
    }
}
int main()
{
    try
    {
        unsigned int sampleRate = 48000;
        auto chain = std::make_shared<EffectChain>();
        chain->setInputGain(3.0f);

        auto distortion = std::make_shared<DistortionEffect>(8.0f, 1.0f);
        auto chorus = std::make_shared<ChorusEffect>(sampleRate);
        auto delay = std::make_shared<DelayEffect>(sampleRate);

        chain->addEffect(distortion, "Distortion", false);
        chain->addEffect(chorus, "Chorus", false);
        chain->addEffect(delay, "Delay", false);

        // Start the user interface in a separate thread
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