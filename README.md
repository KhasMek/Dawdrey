# Dawdrey

A love letter to Audrey II by Infrasonic Audio and Synthux Academy.

Dawdrey is a software port of the Audrey II feedback synthesizer, originally designed by **Synthux Academy** and **Infrasonic Audio**. This plugin brings the "horrorscape" sounds of the hardware unit into your DAW as a VST3 and AU plugin.

## Vibe Coded Notice

I used Google Antigravity and Gemini 3 to assist and expedite the development of this project. While I manually cleaned up and optimized some code, there is definitely more to be done. However, it doesn't use too many CPU cycles or other system resources at this point so I'm fairly happy with it overall. Ultimately, I think it's ethical to notify users that this has been developed using AI assistance and leave it up to the user if they wish to use the software or not. Clearly the original Audrey II code was written by humans...well, at least non-AI organisms. 

## Features

 -   **Feedback Synthesis Engine**: Faithful recreation of the Karplus-Strong inspired feedback loop.
 -   **Stereo Processing**: Independent processing for left and right channels with stereo widening.
 -   **Pitch Shifting**: Integrated pitch shifter in the feedback loop for shimmer and abyss effects.
 -   **Modulation**: 3 LFOs with multiple shapes (Sine, Triangle, Saw, Ramp, Square, Random) and targets.
 -   **Effects**: Built-in Echo and Reverb for spatial depth.
 -   **Input Processing**: Noise Gate and Drive to shape incoming audio.
 -   **Instrument Mode**: Play the resonator like a synthesizer using MIDI notes.
 -   **Preset System**: Save and load your own patches (cross-platform compatible).
 -   **High Feedback**: Like the hardware, this instrument thrives on feedback. Watch your levels, as self-oscillation can get loud quickly!

## Demo

[![Watch the video](https://img.youtube.com/vi/TUwQqKLn37w/maxresdefault.jpg)](https://www.youtube.com/watch?v=TUwQqKLn37w)

## Installation

1. **Copy the Plugin to the Plugin Folder**
    - **macOS:**
        ```bash
        cp Dawdrey.component ~/Library/Audio/Plug-Ins/Components/
        ```
    - **Windows:**
        ```bash
        xcopy /Y /E /I build\Release\Dawdrey.dll "C:\Program Files\Common Files\VST3\"
        ```
    - **Linux:**
        ```bash
        cp build/Release/Dawdrey.so /usr/lib/x86_64-linux-gnu/jack/
        ```
2. **Restart Your DAW**

## Original Hardware

This project is a fan-made port and is **not affiliated with, endorsed by, or supported by Synthux Academy or Infrasonic Audio**.

To support the creators that designed the hardware this plugin is based on, please visit:

 -   **Synthux Academy**: [https://synthux.academy](https://synthux.academy)
 -   **Infrasonic Audio**: [https://infrasonicaudio.com](https://infrasonicaudio.com)

## Credits

 -   **Original Idea and Firmware Design**: Nick Donaldson (Infrasonic Audio)
 -   **Hardware Design and Hardware Release**: Roey (Synthux Academy)
 -   **Original Source Code**: [https://github.com/Synthux-Academy/simple-designer-instruments/tree/main/official/audrey-ii](https://github.com/Synthux-Academy/simple-designer-instruments/tree/main/official/audrey-ii)

This project uses the original DSP code as a base, adapted for the JUCE framework. Huge thanks to Nick and Roey for open-sourcing their work and inspiring this project.

## Local Development

1. **Clone the Repository**
   ```bash
   git clone https://github.com/KhasMek/Dawdrey.git
   cd Dawdrey
   ```

2. **Initialize Submodules**
   If the project uses submodules for JUCE or other dependencies:
   ```bash
   git submodule update --init --recursive
   ```

3. **Configure CMake, Build and Install**
   Create a build directory and generate the build files:
   ```bash
   cmake -B build -S . && cmake --build build --config Release
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   ```
