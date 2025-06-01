#include "RtAudio.h"
#include <iostream>
#include <cmath>
#include <thread>
#include <chrono>
#include <map>
#include <string>

#ifndef PI
#define PI 3.14159265358979323846
#endif

float targetFrequency = 82.41f; // domyślnie E2

float detectFrequencyAutocorrelation(const float *samples, int bufferSize, int sampleRate)
{
    float minExpected = targetFrequency * 0.8f;
    float maxExpected = targetFrequency * 1.25f;
    int minLag = static_cast<int>(sampleRate / maxExpected);
    int maxLag = static_cast<int>(sampleRate / minExpected);

    float maxCorr = 0.0f;
    int bestLag = -1;

    for (int lag = minLag; lag <= maxLag; ++lag)
    {
        float sum = 0.0f;
        for (int i = 0; i < bufferSize - lag; ++i)
            sum += samples[i] * samples[i + lag];

        if (sum > maxCorr)
        {
            maxCorr = sum;
            bestLag = lag;
        }
    }

    if (bestLag > 0)
        return static_cast<float>(sampleRate) / bestLag;
    else
        return 0.0f;
}

// Obliczenie odchylenia w centach
float centsDifference(float detected, float target)
{
    return 1200.0f * log2f(detected / target);
}

int audioCallback(void *outputBuffer, void *inputBuffer,
                  unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status, void *userData)
{
    if (status)
        std::cerr << "Stream error!" << std::endl;

    float *input = static_cast<float *>(inputBuffer);
    static int frameCount = 0;

    if (++frameCount >= 20)
    {
        float freq = detectFrequencyAutocorrelation(input, nBufferFrames, 48000);

        if (freq > targetFrequency * 0.5f && freq < targetFrequency * 2.0f) // ograniczenie dla gitary
        {
            float cents = centsDifference(freq, targetFrequency);
            std::cout << "Częstotliwość: " << freq << " Hz | ";
            if (std::abs(cents) < 10.0f)
                std::cout << "✅ OK (" << cents << " centów)" << std::endl;
            else if (cents > 0)
                std::cout << "🔺 Za wysoko! (+" << cents << " centów)" << std::endl;
            else
                std::cout << "🔻 Za nisko! (" << cents << " centów)" << std::endl;
        }

        frameCount = 0;
    }

    return 0;
}

int main()
{
    // Mapowanie nazw strun
    std::map<std::string, float> stringFrequencies = {
        {"E2", 82.41f}, {"A2", 110.00f}, {"D3", 146.83f}, {"G3", 196.00f}, {"B3", 246.94f}, {"E4", 329.63f}};

    std::string input;
    std::cout << "🎸 Wybierz strunę do strojenia (E2, A2, D3, G3, B3, E4): ";
    std::cin >> input;

    if (stringFrequencies.count(input))
    {
        targetFrequency = stringFrequencies[input];
        std::cout << "🎯 Strojenie struny " << input << " (" << targetFrequency << " Hz)" << std::endl;
    }
    else
    {
        std::cerr << "❌ Nieznana struna, domyślnie E2 (82.41 Hz)" << std::endl;
    }

    try
    {
        RtAudio audio(RtAudio::WINDOWS_WASAPI);
        if (audio.getDeviceCount() < 1)
        {
            std::cerr << "Brak urządzeń audio!" << std::endl;
            return 1;
        }

        unsigned int sampleRate = 48000;
        unsigned int bufferFrames = 2048;

        RtAudio::StreamParameters inputParams;
        inputParams.deviceId = audio.getDefaultInputDevice();
        inputParams.nChannels = 1;
        inputParams.firstChannel = 0;

        std::cout << "🎧 Urządzenie: " << audio.getDeviceInfo(inputParams.deviceId).name << std::endl;
        audio.openStream(nullptr, &inputParams, RTAUDIO_FLOAT32,
                         sampleRate, &bufferFrames, &audioCallback, nullptr);

        std::cout << "▶️ Nasłuchiwanie mikrofonu (Ctrl+C aby zakończyć)..." << std::endl;
        audio.startStream();

        while (true)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (const std::exception &e)
    {
        std::cerr << "Błąd: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
