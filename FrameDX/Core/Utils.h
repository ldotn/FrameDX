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

}