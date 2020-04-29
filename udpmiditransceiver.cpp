#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <enet/enet.h>

#include "RtMidi.h"

#include "MIDI2STR.hpp"

/*
	Tested using GCC version 5.4.0. Requires libraries rtmidi-4.0.0 and enet-1.3.15. Be sure you have ran their configure & make scripts. 

	RtMidi:
	  - https://www.music.mcgill.ca/~gary/rtmidi/
	  - https://github.com/thestk/rtmidi

	ENet:
	  - http://enet.bespin.org/
	
	For enet make sure the shared libraries are in the linker path if you get missing reference errors during runtime.

	Linux: Compilation can be done in using (assuming rtmidi-4.0.0 located in the current folder where this source file is:

	  c++ -Irtmidi-4.0.0 udpmiditransceiver.cpp /usr/local/lib/libenet.so /usr/local/lib/librtmidi.so -o udpmiditransceiver

	Windows / Visual Studio 2019:
	  - Add preprocessor directives __WINDOWS_MM__ and _WIN32
	  - Add additional linker dependencies winmm.lib, enet.lib, ws2_32.lib
	  - Be sure to include rtmidi header and source file to the project
	  - Be sure to have ENet library object files in additional library directories

	 TODO:
	   - Test that bidirectional transfer of MIDI messages works
	   - Change usage from typing name of a MIDI instrument to referring to its port number? Issues might include the ordering changing for some reason
	   - Test effect of polling duration and set default value to sensible?

	-hell1
*/




// These parameters are set globally -- poor taste
RtMidiIn* MIDIin = 0;
RtMidiOut* MIDIout = 0;
ENetPeer* Peer;

// These could be improved / made fail-safe / replaced with some library
bool isoption(int argc, char* argv[], std::string option);
std::string getoptionvalue(int argc, char* argv[], std::string option);

void PrintMIDIDevices();
unsigned int GetMIDIPort(std::string name);

void MIDICallback(double deltatime, std::vector< unsigned char >* message, void* userData); 
void MIDICallbackSilent(double deltatime, std::vector< unsigned char >* message, void* userData);

int main(int argc, char* argv[]) {
	printf("udpmiditransceiver\n\n");

	// Initialize RtMIDI interfaces
	try {
		MIDIin = new RtMidiIn();
	}
	catch (RtMidiError& error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}

	try {
		MIDIout = new RtMidiOut();
	}
	catch (RtMidiError& error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}

	// Initialize ENet
	if (enet_initialize() != 0) {
		printf("ENet initialization failed!\n");
		exit(EXIT_FAILURE);
	}
	else atexit(enet_deinitialize);

	// Options that need to be set, some initiated to default value
	bool UseIn = false;
	unsigned int PortIn;
	std::string DeviceIn;

	bool UseOut = false;
	std::string HostOut;
	unsigned int PortOut;
	std::string DeviceOut;

	unsigned int PollingTime = 10;

	bool IgnoreTiming = true;
	bool IgnoreSensing = true;
	bool IgnoreSysex = true;

	bool PrintMidi = false;

	// These checkups could be improved / made fail-safe / replaced with some library

	if (isoption(argc, argv, "-port-in")) {
		PortIn = atoi(getoptionvalue(argc, argv, "-port-in").c_str());
		if (isoption(argc, argv, "-device-in")) {
			DeviceIn = getoptionvalue(argc, argv, "-device-in");
			UseIn = true;
		}
	}

	if (isoption(argc, argv, "-host-out")) {
		HostOut = getoptionvalue(argc, argv, "-host-out");
		if (isoption(argc, argv, "-port-out")) {
			PortOut = atoi(getoptionvalue(argc, argv, "-port-out").c_str());
			if (isoption(argc, argv, "-device-out")) {
				DeviceOut = getoptionvalue(argc, argv, "-device-out");
				UseOut = true;
			}
		}
	}

	if (isoption(argc, argv, "-polling-time")) PollingTime = atoi(getoptionvalue(argc, argv, "-polling-time").c_str());

	if (isoption(argc, argv, "-timing")) IgnoreTiming = false;
	if (isoption(argc, argv, "-sensing")) IgnoreSensing = false;
	if (isoption(argc, argv, "-sysex")) IgnoreSysex = false;
	if (isoption(argc, argv, "-print-midi")) PrintMidi = true;

	if (isoption(argc, argv, "-print-devices")) PrintMIDIDevices();

	bool DisplayHelp = !UseIn && !UseOut;
	if (DisplayHelp) {
		printf("Following swithces can be used:\n");
		printf("\n");
		printf("  -port-in [integer]       Defines from which UDP port to receive MIDI signal\n");
		printf("  -device-in [string]      Defines which MIDI device receives the signal (input port list)\n");
		printf("\n");
		printf("  -host-out [string]       Defines (ip) address of the device to send MIDI signal to\n");
		printf("  -port-out [integer]      Defines to which UDP port to send MIDI signal to\n");
		printf("  -device-out [string]     Defines which MIDI device sends the signal (output port list)\n");
		printf("\n");
		printf("  -polling-time [number]   Defines UDP polling time in milliseconds (default 10)\n");
		printf("\n");
		printf("  -timing                  Enables receiving timing related MIDI messages (default disabled)\n");
		printf("  -sensing                 Enables receiving sensing related MIDI messages (default disabled)\n");
		printf("  -sysex                   Enables receiving system extension related MIDI messages (default disabled)\n");
		printf("\n");
		printf("  -print-midi              Prints MIDI signals received or sent (default disabled)\n");
		printf("  -print-devices           Print available MIDI devices\n");
		printf("\n");
		printf("At least port-in and device-in, or host-out, port-out, device-out have to be provided.\n");
		printf("\n");
		printf("Usage examples:\n");
		printf("\n");
		printf("  Streaming MIDI from one computer to another:\n");
		printf("    server:   udpmiditransceiver -port-in 6666 -device-in \"loopMIDI Port 1\" -print-midi -print-devices -polling-time 1\n");
		printf("    client:   udpmiditransceiver -host-out 192.168.1.110 -port-out 6666 -device-out \"\"\n");
		printf("\n");
		return(EXIT_SUCCESS);
	}

	printf("Running with parameters:\n");
	if (UseIn) printf(" - Receive MIDI messages through port %d to device '%s'\n", PortIn, DeviceIn.c_str());
	if (UseOut) printf(" - Send MIDI messages to host '%s' port %d from device '%s'\n", HostOut.c_str(), PortOut, DeviceOut.c_str());
	printf(" - Use UDP polling time %d ms\n", PollingTime);
	if (!IgnoreTiming) printf(" - Receive timing related MIDI messages\n");
	if (!IgnoreSensing) printf(" - Receive sensing related MIDI messages\n");
	if (!IgnoreSysex) printf(" - Receive system extension related MIDI messages\n");
	if (PrintMidi) printf(" - Print receive/sent MIDI messages\n");
	printf("\n");

	// Find MIDI port numbers and open them. Note: it can be confusing that when we have "UseIn" (i.e. we are expecting to receive MIDI messages) that we use MIDIout where we will send these messages
	if (UseIn) {
		try {
			bool PortFound = false;
			int MIDIPort = -1;
			int outports = MIDIout->getPortCount();
			for (int port = 0; port < outports; port++) {
				if (MIDIout->getPortName(port).compare(DeviceIn) == 0) {
					PortFound = true;
					MIDIPort = port;
					break;
				}
			}
			if (!PortFound) {
				printf("MIDI out port '%s' not found!", DeviceIn.c_str());
				return(EXIT_FAILURE);
			}
			printf("MIDI output port '%s' found at %d\n", DeviceIn.c_str(), MIDIPort);
			MIDIout->openPort(MIDIPort);
		}
		catch (RtMidiError& error) {
			error.printMessage();
			exit(EXIT_FAILURE);
		}
	}

	if (UseOut) {
		try {
			bool PortFound = false;
			int MIDIPort = -1;
			int inports = MIDIin->getPortCount();
			for (int port = 0; port < inports; port++) {
				if (MIDIin->getPortName(port).compare(DeviceOut) == 0) {
					PortFound = true;
					MIDIPort = port;
					break;
				}
			}
			if (!PortFound) {
				printf("MIDI in port '%s' not found!", DeviceOut.c_str());
				return(EXIT_FAILURE);
			}
			printf("MIDI in port '%s' found at %d\n", DeviceOut.c_str(), MIDIPort);
			MIDIin->openPort(MIDIPort);
			MIDIin->ignoreTypes(IgnoreSysex, IgnoreTiming, IgnoreSensing);
		}
		catch (RtMidiError& error) {
			error.printMessage();
			exit(EXIT_FAILURE);
		}
	}

	// Open inward and outward UDP connections
	ENetAddress AddressIn;
	ENetHost* Server = NULL;
	if (UseIn) {
		AddressIn.host = ENET_HOST_ANY;
		AddressIn.port = PortIn;
		Server = enet_host_create(&AddressIn, 1, 1, 0, 0);
		if (Server == NULL) {
			printf("ENet server initialization failed!\n");
			exit(EXIT_FAILURE);
		}
		printf("Inward UDP ports open\n");
	}

	ENetAddress AddressOut;
	ENetHost* Client = NULL;
	if (UseOut) {
		enet_address_set_host(&AddressOut, HostOut.c_str());
		AddressOut.port = PortOut;
		Client = enet_host_create(NULL, 1, 1, 0, 0);
		if (Client == NULL) {
			printf("ENet client host intialization failed!\n");
			exit(EXIT_FAILURE);
		}
	}

	// Principal loop
	printf("Starting communication loop...\n");
	//bool InwardConnection = false; // Commented out for not being needed
	bool OutwardConnection = false;
	ENetEvent EventIn, EventOut;
	while (1) { 

		// Attempt connection to outward world
		if (UseOut) {
			if (!OutwardConnection) {
				// Attempt connection to outward server
				Peer = enet_host_connect(Client, &AddressOut, 1, 0);
				if (Peer == NULL) {
					printf("ENet connection to peer failed!\n");
					exit(EXIT_FAILURE);
				}
				printf(" - Attempting to connect to server %s:%d\n", HostOut.c_str(), PortOut);
				if(enet_host_service(Client, &EventOut, 1000) > 0 && EventOut.type == ENET_EVENT_TYPE_CONNECT){
					printf(" - Connected to server %s:%d\n", HostOut.c_str(), PortOut);
					OutwardConnection = true;
					if (PrintMidi) MIDIin->setCallback(&MIDICallback);
					else MIDIin->setCallback(&MIDICallbackSilent);
				}
				else {
					printf(" - Failed to connect\n");
					enet_peer_reset(Peer);
				}
			}
			else{
				// If connection established just maintain link and let callback do its things
				if (enet_host_service(Client, &EventOut, PollingTime) > 0) {
					switch (EventOut.type) {
					case ENET_EVENT_TYPE_RECEIVE:
						printf(" - Received message '%s' from server %s:%d\n", (char*) EventOut.packet->data, HostOut.c_str(), PortOut);
						break;
					case ENET_EVENT_TYPE_DISCONNECT:
						printf(" - Server caused disconect\n");
						OutwardConnection = false;
						MIDIin->cancelCallback();
						break;
					default:
						break;
					}
				}
			}
		}

		// Check for inward connections and receive MIDI messages from them
		if (UseIn) {
			if (enet_host_service(Server, &EventIn, PollingTime) > 0){
				switch (EventIn.type) {
				case ENET_EVENT_TYPE_CONNECT:
					// A new inward connection
					{
						char* str = new char[128];
						//sprintf_s(str, 128, "%x:%u", EventIn.peer->address.host, EventIn.peer->address.port);
						std::snprintf(str, 128, "%x:%u", EventIn.peer->address.host, EventIn.peer->address.port);
						EventIn.peer->data = (void*) str;
					}
					printf(" - %s connected\n", (char*) EventIn.peer->data);

					if(1){
						// Send a test message
						char msg[] = "Hello udpmiditransceiver!";
						ENetPacket* packet = enet_packet_create((void*) msg, strlen(msg) + 1, ENET_PACKET_FLAG_RELIABLE);
						enet_peer_send(EventIn.peer, 0, packet);
					}

					//InwardConnection = true;
					break;
				case ENET_EVENT_TYPE_RECEIVE:
					// MIDI messages received
					{
						std::vector<unsigned char> msg;
						unsigned char* data = EventIn.packet->data;
						for (unsigned int n = 0; n < EventIn.packet->dataLength; n++) msg.push_back(data[n]);
						if (PrintMidi) {
							std::string MIDIstr = MIDI2String(msg);
							printf(" - Received %s\n", MIDIstr.c_str());
						}
						MIDIout->sendMessage(&msg);
					}
					enet_packet_destroy(EventIn.packet);
					break;
				case ENET_EVENT_TYPE_DISCONNECT:
					// Inward connection disconnected
					printf(" - %s disconnected\n", (char*) EventIn.peer->data);
					delete[] (char*) EventIn.peer->data;
					//InwardConnection = false;
					break;
				default:
					break;
				}
			}
		}
	}

	// Cleanup -- We don not really reach here...

	if (UseOut) {
		enet_peer_reset(Peer);
		enet_host_destroy(Client);
	}
	if(UseIn) enet_host_destroy(Server);

	delete MIDIin;
	delete MIDIout;

	return(EXIT_SUCCESS);
}





bool isoption(int argc, char* argv[], std::string option) {
	for (int n = 1; n < argc; n++)
		if (option.compare(argv[n]) == 0)
			return(true);
	return(false);
}

std::string getoptionvalue(int argc, char* argv[], std::string option) {
	for (int n = 1; n < argc - 1; n++)
		if (option.compare(argv[n]) == 0)
			return(argv[n + 1]);
	std::string empty;
	return(empty);
}



void PrintMIDIDevices() {
	unsigned int port;
	unsigned int inports = MIDIin->getPortCount();
	printf("There are %d MIDI devices with output ports (senders):\n", inports);
	for (port = 0; port < inports; port++) printf(" -Port %d: %s\n", port, MIDIin->getPortName(port).c_str());
	printf("\n");
	unsigned int outports = MIDIout->getPortCount();
	printf("There are %d MIDI devices with input ports (receivers):\n", outports);
	for (port = 0; port < outports; port++) printf(" -Port %d: %s\n", port, MIDIout->getPortName(port).c_str());
	printf("\n");
}



void MIDICallback(double deltatime, std::vector<unsigned char>* message, void* userData) {
	unsigned char msg[16];
	unsigned int count = message->size();
	for (unsigned int i = 0; i < count; i++) msg[i] = message->at(i);
	ENetPacket* packet = enet_packet_create((void*)msg, count, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(Peer, 0, packet);
	std::string MIDIstr = MIDI2String(*message);
	printf(" - Sent %s\n", MIDIstr.c_str());
}

void MIDICallbackSilent(double deltatime, std::vector<unsigned char>* message, void* userData) {
	unsigned char msg[16];
	unsigned int count = message->size();
	for (unsigned int i = 0; i < count; i++) msg[i] = message->at(i);
	ENetPacket* packet = enet_packet_create((void*)msg, count, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(Peer, 0, packet);
}