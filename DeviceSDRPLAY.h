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

#ifdef HASSDRPLAY
#include <sdrplay_api.h>
#endif

namespace Device {

	class SettingsSDRPLAY : public DeviceSettings
	{
	private:

		int LNAstate = 5;
		int gRdB = 40;
		bool AGC = false;

	public:

		friend class SDRPLAY;

		void Print();
		void Set(std::string option, std::string arg);
	};

	class SDRPLAY : public Control, public StreamOut<CFLOAT32>
	{

#ifdef HASSDRPLAY

		// FIFO
		std::thread demod_thread;
		static void demod_async_static(SDRPLAY* c);

		std::vector<std::vector<CFLOAT32>> fifo;

		static const int sizeFIFO = 256;
		int head = 0;
		int tail = 0;
		std::atomic<int> count;

		std::mutex fifo_mutex;
		std::condition_variable fifo_cond;

		// output vector
		std::vector<CFLOAT32> output;
		const int buffer_size = 16 * 16384;
		int ptr = 0;

		void Demodulation();

		// SDRPLAY specific
		sdrplay_api_DeviceT device;
		sdrplay_api_DeviceParamsT* deviceParams = NULL;
		sdrplay_api_RxChannelParamsT* chParams;

		static void callback_static(short *xi, short *xq, sdrplay_api_StreamCbParamsT *params,unsigned int numSamples, unsigned int reset, void *cbContext);
		static void callback_event_static(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, sdrplay_api_EventParamsT *params, void *cbContext);

		void callback(short *xi, short *xq, sdrplay_api_StreamCbParamsT *params,unsigned int numSamples, unsigned int reset);
		void callback_event(sdrplay_api_EventT eventId, sdrplay_api_TunerSelectT tuner, sdrplay_api_EventParamsT *params);

	public:

		// Control

		void Play();
		void Pause();

		void setSampleRate(uint32_t);
		void setFrequency(uint32_t);

		std::vector<uint32_t> SupportedSampleRates();

		bool isStreaming();

		virtual bool isCallback() { return true; }

		static void pushDeviceList(std::vector<Description>& DeviceList);
		static int getDeviceCount();

		// Device specific
		void openDevice(uint64_t h);
		void openDevice();

		void setSettings(SettingsSDRPLAY& s);

		// static constructor and data
		static class _API
		{
		public:
			bool running = false; _API(); ~_API(); } _api;

		~SDRPLAY() { sdrplay_api_ReleaseDevice(&device); }
#endif
	};
}