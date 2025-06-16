# 🎸 GuitarFX Tuner

**GuitarFX Tuner** is a desktop application developed as a university project, providing both a digital guitar tuner and a real-time audio effects processor for an electric guitar. The project showcases modular design and real-time signal processing capabilities.

## 📚 Description

The application is divided into two independent modules, each designed for a specific audio task:

### 🎼 Tuner App

The **Tuner App** is a lightweight, real-time chromatic tuner designed to detect pitch from live audio input using frequency domain analysis. It utilizes signal processing techniques to extract the fundamental frequency of an instrument's sound wave and compare it against standard musical notes. The app then provides a frequency readout, helping users tune their instruments with precision. Internally, it analyzes incoming audio buffers, autocorrelation algorithm, and calculates the dominant pitch with reference to tuning standards. The code also emphasizes performance and low-latency response using the RtAudio input stream.

### 🎛 Effects App

The **Effects App** functions as a modular effects processor. It captures incoming audio in real time and applies customizable audio effects: distortion, chorus or delay. The architecture suggests a linear audio signal path where each effect can be toggled or chained. The processing is handled by C++ audio routines, relying on raw manipulation of samples to simulate analog-style effects. Users can toggle modules using keyboard input. This structure showcases real-time audio I/O management, low-level digital signal processing, and UI/UX responsiveness in a multithreaded environment.

Both modules use **RtAudio** to manage cross-platform audio input and output, allowing for direct interaction with audio hardware, with precompiled DLLs aiding runtime execution on Windows.

## 🎧 Audio Hardware & Driver Requirements

To properly capture guitar signal and achieve low-latency audio processing, the application relies on:

- 🎛 **Audio Interface used in development**: [Behringer U-Phoria UM2](https://www.behringer.com/product.html?modelCode=P0AVV)  
  Used to connect an electric guitar directly to the computer via USB.

- 🧰 **ASIO4ALL**: [www.asio4all.org](http://www.asio4all.org)  
  This universal ASIO driver provides low-latency audio input/output on Windows systems. It must be installed and correctly configured to route audio through the Behringer UM2.

## 🪟 How to Run on Windows

1. **Connect Your Guitar**  
   Plug your electric guitar into the **INST** input of the **Behringer UM2** audio interface.

2. **Connect the Interface**  
   Use a USB cable to connect the UM2 to your PC. Ensure it is recognized by the system.

3. **Install and Configure ASIO4ALL**  
   - Download and install [ASIO4ALL](http://www.asio4all.org).
   - Open the **ASIO4ALL control panel**.
   - Set proper Input and Output devices.
   - Disable any unused audio devices that could interfere.

4. **Run the App**  
   Launch a chosen `.exe` file. No additional setup is needed if you are on Windows — the required **RtAudio DLLs** are already provided in the executable folder.

> ⚠️ On other operating systems (Linux/macOS), you may need to manually compile RtAudio and adjust input/output device configurations, as precompiled binaries are Windows-specific.

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
