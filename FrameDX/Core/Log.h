#pragma once
#include "stdafx.h"

namespace FrameDX
{
	enum class LogCategory
	{
		Info,
		Warning,
		Error,
		CriticalError,
		_LogCategoryCount
	};

	#define __MAKE_WIDE(x) L##x
	#define MAKE_WIDE(x) __MAKE_WIDE(x) // Double macro to make it expand x if x is a macro

	// Stores a new entry to the log (thread safe)
#define LogRecordEntry(msg,cat) FrameDX::Log._record(msg,cat,__LINE__,MAKE_WIDE(__FUNCTION__),MAKE_WIDE(__FILE__))
	// Checks an assert and stores to the log if false (thread safe)
#define LogAssert(cond,cat) if(!cond) FrameDX::Log._record(#cond L" != true",cat,__LINE__,MAKE_WIDE(__FUNCTION__),MAKE_WIDE(__FILE__))
	// Checks an HRESULT/StatusCode and if it's not S_OK it stores to the log (thread safe)
#define LogCheck(cond,cat) {auto scode = (FrameDX::StatusCode)cond; if(scode != FrameDX::StatusCode::Ok) FrameDX::Log._record(wstring(#cond L" failed with code ") + FrameDX::StatusCodeToString(scode),cat,__LINE__,MAKE_WIDE(__FUNCTION__),MAKE_WIDE(__FILE__)); }
	// Checks an assert and if false stores to the log and returns the "ret" value. (thread safe)
	// This is a macro, so it can be used to return out of a function on failure
#define LogAssertWithReturn(cond,cat,ret) if(!cond) { FrameDX::Log._record(#cond L" != true",cat,__LINE__,MAKE_WIDE(__FUNCTION__),MAKE_WIDE(__FILE__)); return ret; }
	// Checks an HRESULT/StatusCode and if it's not S_OK it stores to the log and returns the HRESULT converted to StatusCode. (thread safe)
	// This is a macro, so it can be used to return out of a function on failure
#define LogCheckWithReturn(cond,cat) {HRESULT hr = cond; if(hr != S_OK) { auto scode = ( FrameDX::StatusCode)hr; FrameDX::Log._record(wstring(#cond L" failed with code ") + FrameDX::StatusCodeToString(scode) ,cat,__LINE__,MAKE_WIDE(__FUNCTION__),MAKE_WIDE(__FILE__)); return scode; }}
	// Checks an assert and if false stores to the log and triggers a debug break (thread safe)
#define LogAssertWithBreak(cond,cat) if(!cond) { FrameDX::Log._record(#cond L" != true",cat,__LINE__,MAKE_WIDE(__FUNCTION__) ,MAKE_WIDE(__FILE__)); DebugBreak();  }
	// Checks an assert, stores to the log if false and returns the !cond. Can be used inside an if (thread safe)
#define LogAssertAndContinue(cond,cat) [&](){ bool b = cond; if(!b) FrameDX::Log._record(#cond L" != true",cat,__LINE__,MAKE_WIDE(__FUNCTION__),MAKE_WIDE(__FILE__)); return !b; }() 
	// Checks an HRESULT/StatusCode and if it's not S_OK it stores to the log. It returns the HRESULT converted to StatusCode. (thread safe)
	// Can be used inside an if
#define LogCheckAndContinue(cond,cat) [&](){auto scode = ( FrameDX::StatusCode)cond; if(scode !=  FrameDX::StatusCode::Ok) { FrameDX::Log._record(wstring(#cond L" failed with code ") + FrameDX::StatusCodeToString(scode) ,cat,__LINE__,MAKE_WIDE(__FUNCTION__),MAKE_WIDE(__FILE__));} return scode; }()
	
	extern class __log
	{
	public:
		struct Entry
		{
			LogCategory Category;
			wstring Message;

			chrono::system_clock::time_point Timestamp;
			int Line;
			wstring Function;
			wstring File;
		};

		// Need to define the functions outside of the header otherwise you get a duplicated definition on the concurrent_vector for some reason

		void _record(const wstring& Message,LogCategory Category,int Line,const wstring& Function,const wstring& File);

		// Prints the entire log to the supplied stream.
		// The stream can be a file, wcout, or any other wostream.
		void PrintAll(wostream& OutputStream);

		// Equal to PrintAll, but only prints a range of logs instead of all of them
		void PrintRange(size_t Start, size_t End, wostream& OutputStream);
	private:
		concurrency::concurrent_vector<Entry> Records;
		const wchar_t* cat_name[(int)LogCategory::_LogCategoryCount] = { L"Info", L"Warning", L"Error", L"CriticalError" };
	} Log;
}