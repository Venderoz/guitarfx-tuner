#include "tuner.h"
#define BUFFER_SIZE 2048
#define SAMPLE_RATE 48000

GuitarTuner::GuitarTuner() : detector(SAMPLE_RATE), frameCount(0) {}

void GuitarTuner::run()
{
    while (true)
    {
        if (!selectString())
            break;

        try
        {
            RtAudio audio(RtAudio::WINDOWS_ASIO);
            inputParams.deviceId = audio.getDefaultInputDevice();
            inputParams.nChannels = 1;
            inputParams.firstChannel = 1;

            unsigned int bufferSize = BUFFER_SIZE;
            audio.openStream(nullptr, &inputParams, RTAUDIO_FLOAT32, SAMPLE_RATE, &bufferSize, &audioCallbackWrapper, this);
            std::cout << "Tuning '" << currentString << "' (" << targetFreq << " Hz)... Press Enter to stop.\n";
            audio.startStream();

            std::thread([]
                        { std::cin.get(); })
                .join();

            audio.stopStream();
            audio.closeStream();
            std::cout << "Tuning stopped.\n\n";
        }
        catch (const std::exception &e)
        {
            std::cerr << "Audio error: " << e.what() << "\n";
        }
    }
}

int GuitarTuner::audioCallbackWrapper(void *, void *inputBuffer, unsigned int nFrames, double, RtAudioStreamStatus status, void *userData)
{
    return static_cast<GuitarTuner *>(userData)->audioCallback(static_cast<float *>(inputBuffer), nFrames, status);
}

int GuitarTuner::audioCallback(float *input, unsigned int nFrames, RtAudioStreamStatus status)
{
    if (status)
        std::cerr << "Stream warning.\n";

    if (++frameCount < 10)
        return 0;

    // Compute RMS to ignore silence, Root Mean Square — it's a mathematical way of measuring the average energy or amplitude of a signal
    float rms = 0;
    for (unsigned int i = 0; i < nFrames; ++i)
        rms += input[i] * input[i];
    rms = std::sqrt(rms / nFrames);

    if (rms < 0.01f)
    {
        frameCount = 0;
        return 0;
    }

    float detected = detector.detect(input, nFrames);
    if (detected > 20 && detected < 1500)
    {
        std::cout << "Detected: " << detected << " Hz | Target: " << targetFreq << " Hz | ";
        if (std::abs(detected - targetFreq) <= 1.0f)
            std::cout << "\033[32mIn tune\033[0m\n";
        else if (detected > targetFreq)
            std::cout << "\033[31mToo sharp\033[0m\n";
        else
            std::cout << "\033[34mToo flat\033[0m\n";
    }

    frameCount = 0;
    return 0;
}

// Prompt user to choose which string to tune
bool GuitarTuner::selectString()
{
    std::map<std::string, float> strings = {
        {"E2", 82.41f}, {"A2", 110.00f}, {"D3", 146.83f}, {"G3", 196.00f}, {"B3", 246.94f}, {"E4", 329.63f}};

    std::string input;
    while (true)
    {
        std::cout << "\nSelect string to tune (\033[33mE2\033[0m, \033[33mA2\033[0m, \033[33mD3\033[0m, \033[33mG3\033[0m, \033[33mB3\033[0m, \033[33mE4\033[0m), or '\033[33mQ\033[0m' to quit: ";
        std::getline(std::cin, input);

        // Convert input to uppercase for case-insensitive comparison
        for (auto &c : input)
            c = std::toupper(c);

        if (input == "Q")
            return false;

        auto it = strings.find(input);
        if (it != strings.end())
        {
            currentString = input;
            targetFreq = it->second;
            detector.setTarget(targetFreq);
            return true;
        }
        else
        {
            std::cout << "\033[1;31mInvalid string! Try again!\033[0m\n";
        }
    }
}
