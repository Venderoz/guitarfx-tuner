#include <iostream>
#include "RtAudio.h"

class AudioPassthrough {
public:
    AudioPassthrough() {
        if (audio.getDeviceCount() < 1) {
            throw std::runtime_error("No audio devices found.");
        }

        // Input: 1 channel (mono), e.g., guitar or mic
        inputParams.deviceId = audio.getDefaultInputDevice();
        inputParams.nChannels = 1;
        inputParams.firstChannel = 0;

        // Output: 2 channels (stereo headphones/speakers)
        outputParams.deviceId = audio.getDefaultOutputDevice();
        outputParams.nChannels = 2;
        outputParams.firstChannel = 0;
    }

    void start(unsigned int sampleRate = 44100, unsigned int bufferFrames = 256) {
        try {
            audio.openStream(&outputParams, &inputParams, RTAUDIO_FLOAT32,
                             sampleRate, &bufferFrames, &AudioPassthrough::callback, this);
            audio.startStream();
        } catch (std::exception &e) {
            std::cerr << "Error opening stream: " << e.what() << std::endl;
            throw;
        }
    }

    void stop() {
        if (audio.isStreamRunning()) audio.stopStream();
        if (audio.isStreamOpen()) audio.closeStream();
    }

private:
    static int callback(void *outputBuffer, void *inputBuffer,
                        unsigned int nFrames, double /*streamTime*/,
                        RtAudioStreamStatus status, void *userData) {
        if (status) std::cerr << "⚠ Stream underflow/overflow detected." << std::endl;

        float *in = static_cast<float *>(inputBuffer);
        float *out = static_cast<float *>(outputBuffer);

        // Mono to Stereo passthrough
        for (unsigned int i = 0; i < nFrames; ++i) {
            float inputSample = in ? in[i] : 0.0f;
            out[2 * i]     = inputSample; // Left
            out[2 * i + 1] = inputSample; // Right
        }

        return 0;
    }

    RtAudio audio;
    RtAudio::StreamParameters inputParams, outputParams;
};

int main() {
    try {
        AudioPassthrough passthrough;
        passthrough.start();

        std::cout << "🔊 Audio passthrough running. Press Enter to stop...\n";
        std::cin.get();

        passthrough.stop();
    } catch (const std::exception &e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
