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

#include <cstring>

#include "Device.h"
#include "DeviceHACKRF.h"
#include "Common.h"
#include "Utilities.h"

namespace Device {


	void SettingsHACKRF::Print()
	{
		std::cerr << "Hackrf Settings: -gf";
		std::cerr << " preamp ";
		if(preamp) std::cerr << "ON"; else std::cerr << "OFF";
		std::cerr << " lna " << LNA_Gain;
		std::cerr << " vga " << VGA_Gain;
		std::cerr << std::endl;
	}

	void SettingsHACKRF::Set(std::string option, std::string arg)
	{
		Util::Convert::toUpper(option);
		Util::Convert::toUpper(arg);

		if (option == "LNA")
		{
			LNA_Gain = ((Util::Parse::Integer(arg, 0, 40) + 4) / 8) * 8;
		}
		else if (option == "VGA")
		{
			VGA_Gain = ((Util::Parse::Integer(arg, 0, 62) + 1) / 2) * 2;
		}
		else if (option == "PREAMP")
		{
			preamp = Util::Parse::Switch(arg);
		}
		else
			throw "Invalid setting for HACKRF.";

	}

	//---------------------------------------
	// Device HACKRF

#ifdef HASHACKRF

	void HACKRF::Open(uint64_t h, SettingsHACKRF& s)
	{
		int result = hackrf_open(&device);
		if (result != HACKRF_SUCCESS) throw "HACKRF: cannot open device";

		applySettings(s);
	}

	void HACKRF::Open(SettingsHACKRF& s)
	{
		if (hackrf_open(&device) != HACKRF_SUCCESS) throw "HACKRF: cannot open device";

		applySettings(s);
	}

	void HACKRF::setLNA_Gain(int a)
	{
		if (hackrf_set_lna_gain(device, a) != HACKRF_SUCCESS) throw "HACKRF: cannot set LNA gain.";
	}

	void HACKRF::setVGA_Gain(int a)
	{
		if (hackrf_set_vga_gain(device, a) != HACKRF_SUCCESS) throw "HACKRF: cannot set VGA gain.";
	}

	void HACKRF::setPREAMP(int a)
	{
		if (hackrf_set_amp_enable(device, a) != HACKRF_SUCCESS) throw "HACKRF: cannot set amp.";
	}

	void HACKRF::setSampleRate(uint32_t s)
	{
		if (hackrf_set_sample_rate(device, s) != HACKRF_SUCCESS) throw "HACKRF: cannot set sample rate.";
		if (hackrf_set_baseband_filter_bandwidth(device, hackrf_compute_baseband_filter_bw(s)) != HACKRF_SUCCESS) throw "HACKRF: cannot set bandwidth filter to auto.";

		DeviceBase::setSampleRate(s);
	}

	void HACKRF::Close()
	{
		hackrf_close(device);
	}

	void HACKRF::setFrequency(uint32_t f)
	{
		if (hackrf_set_freq(device, f) != HACKRF_SUCCESS) throw "HACKRF: cannot set frequency.";

		DeviceBase::setFrequency(f);
	}

	int HACKRF::callback_static(hackrf_transfer* tf)
	{
		((HACKRF*)tf->rx_ctx)->callback(tf->buffer, tf->valid_length);
		return 0;
	}

	void HACKRF::callback(uint8_t* data, int len)
	{
		int len2 = len / 2;

		if (output.size() < len2) output.resize(len2);

		for (int i = 0; i < len2; i++)
		{
			output[i].real(((int8_t)data[2 * i]) / 128.0f);
			output[i].imag(((int8_t)data[2 * i + 1]) / 128.0f);
		}

		Send((const CFLOAT32*)output.data(), output.size());
	}

	void HACKRF::Play()
	{
		DeviceBase::Play();

		if (hackrf_start_rx(device, HACKRF::callback_static, this)  != HACKRF_SUCCESS) throw "HACKRF: Cannot open device";

		SleepSystem(10);
	}

	void HACKRF::Stop()
	{
		hackrf_stop_rx(device);
		streaming = false;

		DeviceBase::Stop();
	}

	std::vector<uint32_t> HACKRF::SupportedSampleRates()
	{
		return { 6000000 };
	}

	void HACKRF::pushDeviceList(std::vector<Description>& DeviceList)
	{
		std::vector<uint64_t> serials;
		hackrf_device_list_t* list;

		if (hackrf_init() != HACKRF_SUCCESS) throw "Cannot open hackrf library";

		list = hackrf_device_list();
		serials.resize(list->devicecount);

		for (int i = 0; i < list->devicecount; i++)
		{
			if (list->serial_numbers[i])
			{
				std::stringstream serial;
				serial << std::uppercase << list->serial_numbers[i];
				DeviceList.push_back(Description("HACKRF", "HACKRF", serial.str(), (uint64_t)i, Type::HACKRF));
			}
		}
		hackrf_device_list_free(list);
	}

	bool HACKRF::isStreaming()
	{
		return hackrf_is_streaming(device) == HACKRF_TRUE;
	}

	void HACKRF::applySettings(SettingsHACKRF& s)
	{
		setPREAMP(s.preamp ? 1 : 0);
		setLNA_Gain(s.LNA_Gain);
		setVGA_Gain(s.VGA_Gain);
	}

#endif
}
