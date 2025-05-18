#include "RtAudio.h"
#include "fftw3.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>
#include <map>
#include <string>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// ===========================
// Struktura danych do tunera
struct TunerSettings
{
    float targetFrequency;
    std::string noteName;
    float tolerance = 1.0f; // dopuszczalna odchyłka w Hz
};

// ===========================
// Mapa strun gitarowych (standard tuning)
std::map<std::string, TunerSettings> guitarStrings = {
    {"E6", {82.41f, "E6"}}, // najgrubsza
    {"A", {110.00f, "A5"}},
    {"D", {146.83f, "D4"}},
    {"G", {196.00f, "G3"}},
    {"B", {246.94f, "B2"}},
    {"E1", {329.63f, "E1"}} // najcieńsza
};

// ===========================
// Globalnie wybrana struna
TunerSettings currentTarget = guitarStrings["E1"]; // domyślnie E1
float correctToFundamental(float frequency, float target)
{
    const float tolerance = 1.5f;

    for (int h = 2; h <= 6; ++h)
    {
        float divided = frequency / h;
        if (std::abs(divided - target) <= tolerance)
        {
            return divided;
        }
    }

    return frequency; // brak korekty
}
// ===========================
// Wykrywanie częstotliwości (FFT)
float detectFrequencyFFT(float *samples, int bufferSize, int sampleRate = 48000)
{
    fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * (bufferSize / 2 + 1));
    double *in = (double *)fftw_malloc(sizeof(double) * bufferSize);

    // Normalizacja + Hanning window
    for (int i = 0; i < bufferSize; ++i)
    {
        double sample = static_cast<double>(samples[i]);
        if (sample > 1.0)
            sample = 1.0;
        if (sample < -1.0)
            sample = -1.0;

        double window = 0.5 * (1.0 - cos(2.0 * PI * i / (bufferSize - 1)));
        in[i] = sample * window;
    }

    fftw_plan plan = fftw_plan_dft_r2c_1d(bufferSize, in, out, FFTW_ESTIMATE);
    fftw_execute(plan);

    int maxIndex = 0;
    float maxMag = 0.0f;

    int minBin = static_cast<int>(50.0f / ((float)sampleRate / bufferSize)); // np. dla 50Hz

    for (int i = minBin; i < bufferSize / 2; ++i)
    {
        float real = out[i][0];
        float imag = out[i][1];
        float magnitude = std::sqrt(real * real + imag * imag);
        if (magnitude > maxMag)
        {
            maxMag = magnitude;
            maxIndex = i;
        }
    }
    if (maxMag < 0.05f)
    {
        std::cout << "[DEBUG] Za mała amplituda - brak dźwięku?" << std::endl;
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
        return 0.0f;
    }
    if (maxIndex <= 0 || maxIndex >= bufferSize / 2 - 1)
    {
        fftw_destroy_plan(plan);
        fftw_free(in);
        fftw_free(out);
        return 0.0f; // nie można interpolować
    }

    // Diagnostyka
    std::cout << "[DEBUG] maxIndex: " << maxIndex << ", maxMag: " << maxMag << std::endl;

    // Interpolacja paraboliczna z sąsiednimi próbkami
    float magL = std::sqrt(out[maxIndex - 1][0] * out[maxIndex - 1][0] + out[maxIndex - 1][1] * out[maxIndex - 1][1]);
    float magR = std::sqrt(out[maxIndex + 1][0] * out[maxIndex + 1][0] + out[maxIndex + 1][1] * out[maxIndex + 1][1]);

    float delta = 0.5f * (magR - magL) / (2 * maxMag - magL - magR);
    float interpolatedIndex = maxIndex + delta;

    float frequency = interpolatedIndex * ((float)sampleRate / bufferSize);

    frequency = correctToFundamental(frequency, currentTarget.targetFrequency);
    // Filtr zakresu
    if (frequency < 50.0f || frequency > 1000.0f)
        frequency = 0.0f;

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);

    return frequency;
}

// Audio callback
int audioCallback(void *outputBuffer, void *inputBuffer,
                  unsigned int nBufferFrames,
                  double streamTime, RtAudioStreamStatus status, void *userData)
{
    static int frameCounter = 0;

    if (status)
        std::cerr << "Stream underflow/overflow detected!" << std::endl;

    float *input = static_cast<float *>(inputBuffer);

    if (++frameCounter >= 20)
    {
        float freq = detectFrequencyFFT(input, nBufferFrames);
        float delta = freq - currentTarget.targetFrequency;
        if (freq != 0.0f)
        {
            std::cout << "Wykryta czestotliwosc: " << freq << " Hz -> ";
            if (std::abs(delta) < currentTarget.tolerance)
                std::cout << "W STROJU (" << currentTarget.noteName << ")" << std::endl;
            else if (delta > 0)
                std::cout << "ZA WYSOKO" << std::endl;
            else
                std::cout << "ZA NISKO" << std::endl;
        }

        frameCounter = 0;
    }

    return 0;
}

// ===========================
// Główna funkcja
int main()
{
    RtAudio audio(RtAudio::WINDOWS_ASIO);
    unsigned int devices = audio.getDeviceCount();
    for (unsigned int i = 0; i < devices; ++i)
    {
        RtAudio::DeviceInfo info = audio.getDeviceInfo(i);
        std::cout << "ID #" << i << ": " << info.name << std::endl;
        std::cout << "  Input channels: " << info.inputChannels << std::endl;
        std::cout << "  Output channels: " << info.outputChannels << std::endl;
        std::cout << "  Preferred sample rate: " << info.preferredSampleRate << std::endl;
        std::cout << "  Is default input: " << (info.isDefaultInput ? "Yes" : "No") << std::endl;
        std::cout << "  Is default output: " << (info.isDefaultOutput ? "Yes" : "No") << std::endl;
        std::cout << "------------------------" << std::endl;
    }
    std::cout << "Wybierz strune do strojenia (E6, A, D, G, B, E1): ";
    std::string inputNote;
    std::cin >> inputNote;

    if (guitarStrings.find(inputNote) != guitarStrings.end())
    {
        currentTarget = guitarStrings[inputNote];
        std::cout << "Strojenie do: " << currentTarget.noteName
                  << " (" << currentTarget.targetFrequency << " Hz)" << std::endl;
    }
    else
    {
        std::cerr << "Nieprawidłowa nazwa struny!" << std::endl;
        return 1;
    }

    try
    {
        // RtAudio audio(RtAudio::WINDOWS_ASIO);
        if (audio.getDeviceCount() < 1)
        {
            std::cerr << "Nie wykryto żadnych urządzeń audio!" << std::endl;
            return 1;
        }

        unsigned int bufferFrames = 8192;
        unsigned int sampleRate = 48000;

        RtAudio::StreamParameters inputParams;
        inputParams.deviceId = audio.getDefaultInputDevice();
        inputParams.nChannels = 1;
        inputParams.firstChannel = 0;

        RtAudio::StreamOptions options;

        audio.openStream(
            nullptr,
            &inputParams,
            RTAUDIO_FLOAT32,
            sampleRate,
            &bufferFrames,
            &audioCallback,
            nullptr,
            &options);

        std::cout << "Rozpoczynam strojenie struny..." << std::endl;
        audio.startStream();

        std::this_thread::sleep_for(std::chrono::seconds(30));

        audio.stopStream();
        audio.closeStream();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Błąd: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
