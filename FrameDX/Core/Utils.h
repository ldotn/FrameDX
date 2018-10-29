#pragma once
#include "stdafx.h"
#include "Core.h"
#include "../Device/Device.h"

namespace FrameDX
{
	// Loops and calls f, while trying to keep all iterations of the same duration
	// SHOULD NOT BE ASSUMED TO BE ACCURATE!!!
	// It relays on this_thread::sleep_for, so it's only as accurate as sleep_for is
	// Can be interrupted
	// Allows the use of generic std::function functions, which is a big limitation of Win32 timers
	// A quick test based on the log printer loop of the test app gives the following results
	// System : Windows 10 Pro x64 10.0.17134, Xeon E5-2683 V3, 32 GB RAM, 2x Samsung Evo 850 (250 GB, RAID0)
	//		Period | Error (%)
	//		------------------
	//		 10 ms | 9 - 10 
	//		 25 ms | 3.5 - 4
	//		 50 ms | 1.5 - 2
	//		100 ms | 0.5 - 1
	//		200 ms | 0.4 - 0.5
	//		500 ms | 0.18 - 0.2
	template<typename D>
	void TimedLoop(function<void()> f,D d, bool* BreakCondition = nullptr)
	{
		// Find the overhead of sleep
		// Assuming a constant overhead, something that IS WRONG
		auto t0 = chrono::high_resolution_clock::now();
			this_thread::sleep_for(1ns);
		auto t1 = chrono::high_resolution_clock::now();
		auto overhead = t1 - t0;

		while((BreakCondition && *BreakCondition) || !BreakCondition)
		{
			t0 = chrono::high_resolution_clock::now();
				f();
			t1 = chrono::high_resolution_clock::now();
			
			// Don't sleep if the time is under the overhead of sleep_for
			// Also compensate for overhead here
			auto r = d - (t1 - t0) - overhead;
			if(r > overhead)
				this_thread::sleep_for(r);
		}
	}
	
	template<typename T>
	T ceil(T x,T y){ return x/y + (x % y != 0); }

	template<typename T>
	T clamp(T x, T a, T b) { return x < a ? a : (x > b ? b : x); }

	template<typename T>
	T saturate(T x) { return clamp(x, T(0), T(1));  }

	#define __unique_string_inner2(str,c) str #c
	#define __unique_string_inner(str,c) __unique_string_inner2(str,c)
	#define UNIQUE_STRING(base) __unique_string_inner( base, __COUNTER__ )

	// Helper to create a D3D buffer from a vector. Requires the object to have a size() member.
	template<typename T>
	StatusCode CreateBufferFromVector(vector<T> const& DataVector, Device& Dev, D3D11_BIND_FLAG BindFlags, ID3D11Buffer** OutBuffer,D3D11_USAGE Usage = D3D11_USAGE_IMMUTABLE,const string& Name = UNIQUE_STRING("FrameDX:VectorBuffer") )
	{
		D3D11_BUFFER_DESC desc = {};

		desc.ByteWidth = (UINT)DataVector.size() * sizeof(T);
		desc.BindFlags = BindFlags;
		desc.Usage = Usage;

		D3D11_SUBRESOURCE_DATA data_desc = {};
		data_desc.pSysMem = &DataVector[0];

		*OutBuffer = nullptr;
		auto s = LogCheckAndContinue(Dev.GetDevice()->CreateBuffer(&desc,&data_desc,OutBuffer),LogCategory::Error);
		if(s == StatusCode::Ok)
			(*OutBuffer)->SetPrivateData(WKPDID_D3DDebugObjectName, Name.length(), Name.c_str());

		return s;
	}
	
	// User-defined literal to write degrees to radians
	constexpr long double operator"" _deg(long double x)
	{
		return (x * 3.1415926535897932384626433832795) / 180.0;
	}

	constexpr long double operator"" _deg(unsigned long long int x)
	{
		return (((long double)x) * 3.1415926535897932384626433832795) / 180.0;
	}
}