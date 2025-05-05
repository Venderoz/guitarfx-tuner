#include "RtAudio.h"
#include <iostream>

int main() {
    try {
        RtAudio audio(RtAudio::WINDOWS_WASAPI);

        std::cout << "Using API: " << audio.getCurrentApi() << std::endl;

        unsigned int defaultInput = audio.getDefaultInputDevice();
        std::cout << "Default input device ID: " << defaultInput << std::endl;

        RtAudio::DeviceInfo info = audio.getDeviceInfo(defaultInput);

        std::cout << "Device Name: " << info.name << std::endl;
        std::cout << "Input Channels: " << info.inputChannels << std::endl;

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
