# 🎸 GuitarFX Tuner

**GuitarFX Tuner** is a desktop application developed as a university project, providing both a digital guitar tuner and a real-time audio effects processor for an electric guitar. The project showcases modular design and real-time signal processing capabilities.

## 📚 Description

The application is divided into two independent modules, each designed for a specific audio task:

### 🎼 Tuner App

The **Tuner App** is a lightweight, real-time chromatic tuner designed to detect pitch from live audio input using frequency domain analysis. It utilizes signal processing techniques to extract the fundamental frequency of an instrument's sound wave and compare it against standard musical notes. The app then provides a frequency readout, helping users tune their instruments with precision. Internally, it analyzes incoming audio buffers, autocorrelation algorithm, and calculates the dominant pitch with reference to tuning standards. The code also emphasizes performance and low-latency response using the RtAudio input stream.

### 🎛 Effects App

The **Effects App** functions as a modular effects processor. It captures incoming audio in real time and applies customizable audio effects: distortion, chorus or delay. The architecture suggests a linear audio signal path where each effect can be toggled or chained. The processing is handled by C++ audio routines, relying on raw manipulation of samples to simulate analog-style effects. Users can toggle modules using keyboard input. This structure showcases real-time audio I/O management, low-level digital signal processing, and UI/UX responsiveness in a multithreaded environment.

Both modules use **RtAudio** to manage cross-platform audio input and output, allowing for direct interaction with audio hardware, with precompiled DLLs aiding runtime execution on Windows.

## 🧪 Technologies Used

- **Language**: C++
- **Audio Library**: [RtAudio](https://github.com/thestk/rtaudio)
- **Build Tools**: CMake (for library compiling)
- **Platform**: Windows (precompiled DLLs included)

## 👤 Authors

Developed by 
**Ivan Soboliev** ([@Venderoz](https://github.com/Venderoz)) 
and
**Bohdan Sklianchuk** ([@sklianchukk](https://github.com/sklianchukk)) 
as part of a university software engineering project.
