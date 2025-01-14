/*
Copyright(c) 2021 jvde.github@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "Device.h"

#ifdef HASHACKRF
#include <libhackrf/hackrf.h>
#endif

namespace Device {


	class SettingsHACKRF : public DeviceSettings
	{
	private:

		int LNA_Gain = 8;
		int VGA_Gain = 20;
		bool preamp = false;

	public:

		friend class HACKRF;

		void Print();
		void Set(std::string option, std::string arg);
	};

	class HACKRF : public DeviceBase, public StreamOut<CFLOAT32>
	{
#ifdef HASHACKRF

		hackrf_device* device = NULL;

		void setLNA_Gain(int);
		void setVGA_Gain(int);
		void setPREAMP(int);

		static int callback_static(hackrf_transfer* tf);
		void callback(uint8_t*, int);

		std::vector<CFLOAT32> output;

	public:

		// Control

		void Play();
		void Stop();

		void setSampleRate(uint32_t);
		void setFrequency(uint32_t);

		std::vector<uint32_t> SupportedSampleRates();

		bool isStreaming();

		virtual bool isCallback() { return true; }

		static void pushDeviceList(std::vector<Description>& DeviceList);

		// Device specific
		void Open(uint64_t h, SettingsHACKRF& s);
		void Open(SettingsHACKRF& s);
		void Close();

		void applySettings(SettingsHACKRF& s);
#endif
	};
}
