#pragma once
#include "stdafx.h"

namespace FrameDX
{
	// Make varadic later
	template<template <typename,typename> typename C0, template <typename,typename> typename C1, typename T0, typename T1, typename _alloc0 = allocator<T0>, typename _alloc1 = allocator<T1>>
	class __zip
	{
	public:
		class InternalIterator
		{
		public:
			InternalIterator(int CountV,int CurrentV,typename  C0<T0,_alloc0>::iterator InIterA,typename  C1<T1,_alloc1>::iterator InIterB)
				: IterA(InIterA), IterB(InIterB)
			{
				Current = CurrentV;
				Count = CountV;
			}

			InternalIterator operator++()
			{
				Current++;
				IterA++;
				IterB++;
				return *this;
			}

			inline bool operator!=(const InternalIterator& rhs){ return !(Current == rhs.Current); }

			tuple<T0*,T1*> operator*() { return make_tuple(&(*IterA),&(*IterB)); }
		private:
			int Count;
			int Current;
			typename C0<T0,_alloc0>::iterator IterA;
			typename C1<T1,_alloc1>::iterator IterB;
		};

		__zip(C0<T0,_alloc0>& a, C1<T1,_alloc1>& b) 
			: RefA(a), RefB(b)
		{
			auto s_a = std::size(a);
			auto s_b = std::size(b);
			Count = s_a < s_b ? s_a : s_b;
		}

		InternalIterator begin() { return InternalIterator(Count,0,RefA.begin(),RefB.begin()); }
		InternalIterator end() { return InternalIterator(Count,Count,RefA.end(),RefB.end()); }
	private:
		int Count;
		C0<T0,_alloc0>& RefA;
		C1<T1,_alloc1>& RefB;
	};

	template<template <typename,typename> typename C0, template <typename,typename> typename C1, typename T0, typename T1, typename _alloc0 = allocator<T0>, typename _alloc1 = allocator<T1>>
	__zip<C0,C1,T0,T1,_alloc0,_alloc1> zip(C0<T0,_alloc0>& a, C1<T1,_alloc1>& b){ return __zip<C0,C1,T0,T1>(a,b); }
	

	// Loops and calls f, while trying to keep all iterations of the same duration
	// Can be interrupted
	template<typename D>
	void TimedLoop(function<void()> f,D d, bool* BreakCondition = nullptr)
	{
		while((BreakCondition && *BreakCondition) || !BreakCondition)
		{
			auto t0 = chrono::high_resolution_clock::now();
				f();
			auto t1 = chrono::high_resolution_clock::now();
			
			this_thread::sleep_for(d - (t1 - t0));
		}
	}
	
	template<typename T>
	T ceil(T x,T y){ return x/y + (x % y != 0); }


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

}