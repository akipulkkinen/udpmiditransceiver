# udpmiditransceiver
Send/receive MIDI messages over UDP connection with source/target being e.g. physical or virtual MIDI device

udpmiditransceiver was made to enable transfering MIDI messages from Linux/Windows connected device to another Linux/Windows MIDI device over WIFI or LAN using UDP protocol.

udpmiditransceiver has been tested using GCC version 5.4.0 and Visual Studio 2019. It requires libraries rtmidi-4.0.0 and enet-1.3.15.

RtMidi:
- https://www.music.mcgill.ca/~gary/rtmidi/
- https://github.com/thestk/rtmidi

ENet:
- http://enet.bespin.org/
	
For ENet make sure the shared libraries are in the linker path if you get missing reference errors during runtime.

Linux: Compilation can be done in using (assuming rtmidi-4.0.0 located in the current folder where this source file is:
- c++ -Irtmidi-4.0.0 udpmiditransceiver.cpp /usr/local/lib/libenet.so /usr/local/lib/librtmidi.so -o udpmiditransceiver

Windows / Visual Studio 2019:
- Add preprocessor directives __WINDOWS_MM__ and _WIN32
- Add additional linker dependencies winmm.lib, enet.lib, ws2_32.lib
- Be sure to include rtmidi header and source file to the project
- Be sure to have ENet library object files in additional library directories

TODO:
- Test that bidirectional transfer of MIDI messages works
- Test effect of polling duration and set default value to sensible?

-hell1
