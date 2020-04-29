#ifndef __MIDI2STR_HPP__
#define __MIDI2STR_HPP__

#include <sstream>

// Implementation below follows 
// https://users.cs.cf.ac.uk/Dave.Marshall/Multimedia/node158.html
// https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message
// https://www.songstuff.com/recording/article/midi_message_format/

// Translates MIDI message into human readable text
std::string MIDI2String(std::vector<unsigned char>& message) {
	std::ostringstream str;

	int status = message.at(0);
	int status_high = status & 0xF0;

	switch (status_high) {

	case 0x80:
	{
		int channel = message.at(0) & 0x0F;
		int key = message.at(1);
		int velocity = message.at(2);
		str << "Note off, channel " << channel << ", key " << key << ", velocity " << velocity;
	}
	break;

	case 0x90:
	{
		int channel = message.at(0) & 0x0F;
		int key = message.at(1);
		int velocity = message.at(2);
		str << "Note on, channel " << channel << ", key " << key << ", velocity " << velocity;
	}
	break;

	case 0xA0:
	{
		int channel = message.at(0) & 0x0F;
		int key = message.at(1);
		int pressure = message.at(2);
		str << "Polyphonic key pressure, channel " << channel << ", key " << key << ", pressure " << pressure;
	}
	break;

	case 0xB0:
	{
		int channel = message.at(0) & 0x0F;
		int controller = message.at(1);
		int value = message.at(2);
		switch (controller) {
		case 0x01:
			str << "Control change, Modulation wheel, channel " << channel << ", value " << value;
			break;
		case 0x02:
			str << "Control change, Breath controller, channel " << channel << ", value " << value;
			break;
		case 0x04:
			str << "Control change, Foot controller, channel " << channel << ", value " << value;
			break;
		case 0x05:
			str << "Control change, Portamento time, channel " << channel << ", value " << value;
			break;
		case 0x06:
			str << "Control change, Data entry slider, channel " << channel << ", value " << value;
			break;
		case 0x07:
			str << "Control change, Main volume, channel " << channel << ", value " << value;
			break;

		case 0x21:
			str << "Control change, Modulation wheel, channel " << channel << ", value " << value;
			break;
		case 0x22:
			str << "Control change, Breath controller, channel " << channel << ", value " << value;
			break;
		case 0x24:
			str << "Control change, Foot controller, channel " << channel << ", value " << value;
			break;
		case 0x25:
			str << "Control change, Portamento time, channel " << channel << ", value " << value;
			break;
		case 0x26:
			str << "Control change, Data entry slider, channel " << channel << ", value " << value;
			break;
		case 0x27:
			str << "Control change, Main volume, channel " << channel << ", value " << value;
			break;

		case 0x40:
			str << "Control change, Sustain pedal, channel " << channel << ", value " << value;
			break;
		case 0x41:
			str << "Control change, Portamento, channel " << channel << ", value " << value;
			break;
		case 0x42:
			str << "Control change, Sostenato pedal, channel " << channel << ", value " << value;
			break;
		case 0x43:
			str << "Control change, Soft pedal, channel " << channel << ", value " << value;
			break;

		case 0x60:
			str << "Control change, Data increment, channel " << channel << ", value " << value;
			break;
		case 0x61:
			str << "Control change, Data decrement, channel " << channel << ", value " << value;
			break;
		case 0x62:
			str << "Control change, Non-registered parameter number, channel " << channel << ", LSB " << value;
			break;
		case 0x63:
			str << "Control change, Non-registered parameter number, channel " << channel << ", MSB " << value;
			break;
		case 0x64:
			str << "Control change, Registered parameter number, channel " << channel << ", LSB " << value;
			break;
		case 0x65:
			str << "Control change, Registered parameter number, channel " << channel << ", MSB " << value;
			break;

		case 0x79:
			str << "Control change, Channel mode, channel " << channel << ", Reset all controllers";
			break;
		case 0x7A:
			str << "Control change, Channel mode, channel " << channel << ", Local control, value " << value;
			break;
		case 0x7B:
			str << "Control change, Channel mode, channel " << channel << ", All notes off";
			break;
		case 0x7C:
			str << "Control change, Channel mode, channel " << channel << ", Omni mode off";
			break;
		case 0x7D:
			str << "Control change, Channel mode, channel " << channel << ", Omni mode on";
			break;
		case 0x7E:
			str << "Control change, Channel mode, channel " << channel << ", Mono mode on (Poly mode off), value " << value;
			break;
		case 0x7F:
			str << "Control change, Channel mode, channel " << channel << ", Poly mode on (Mono mode off)";
			break;

		default:
			str << "Control change, channel " << channel << ", controller " << controller << ", value " << value;
			break;
		}
	}
	break;

	case 0xC0:
	{
		int channel = message.at(0) & 0x0F;
		int program = message.at(1);
		str << "Program change , channel " << channel << ", program " << program;
	}
	break;

	case 0xD0:
	{
		int channel = message.at(0) & 0x0F;
		int pressure = message.at(1);
		str << "Channel aftertouch, channel " << channel << ", pressure " << pressure;
	}
	break;

	case 0xE0:
	{
		int channel = message.at(0) & 0x0F;
		int MSB = message.at(1);
		int LSB = message.at(2);
		str << "Pitch bend, channel " << channel << ", MSB " << MSB << ", LSB " << LSB;
	}
	break;

	case 0xF0:
	{
		int status_low = message.at(0) & 0x0F;
		switch (status_low) {
		case 0x08:
			str << "System real-time, Timing clock";
			break;
		case 0x0A:
			str << "System real-time, Start sequence";
			break;
		case 0x0B:
			str << "System real-time, Continue sequence";
			break;
		case 0x0C:
			str << "System real-time, Stop sequence";
			break;
		case 0x0E:
			str << "System real-time, Active sensing";
			break;
		case 0x0F:
			str << "System real-time, System reset";
			break;

		case 0x01:
		{
			int value = message.at(1);
			str << "System common, MIDI timing code, value " << value;
		}	break;
		case 0x02:
		{
			int value1 = message.at(1);
			int value2 = message.at(2);
			str << "System common, Song position pointer, values " << value1 << ", " << value2;
		}	break;
		case 0x03:
		{
			int value = message.at(1);
			str << "System common, Song select, value " << value;
		}
		break;
		case 0x06:
			str << "System common, Tune request";
			break;

		case 0x00:
			str << "System exclusive start";
			break;

		case 0x07:
			str << "System exclusive end";
			break;

		default:
			str << "Unkown message status " << status;
			break;
		}
	}
	break;

	default:
		str << "Unknown message status" << status;
		break;
	}
	return(str.str());
}


#endif