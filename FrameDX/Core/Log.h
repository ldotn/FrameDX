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

	// Stores a new entry to the log (thread safe)
#define LogRecordEntry(msg,cat) Log::_record(msg,cat,__LINE__,__func__,__FILE__)
	// Checks an assert and stores to the log if false (thread safe)
#define LogAssert(cond,cat) if(!cond) Log::_record(#cond L" != true",cat,__LINE__,__func__,__FILE__)
	// Checks an assert and if false stores to the log and returns the "ret" value. (thread safe)
	// This is a macro, so it can be used to return out of a function on failure
#define LogAssertWithReturn(cond,cat,ret) if(!cond) { Log::_record(#cond L" != true",cat,__LINE__,__func__,__FILE__); return ret; }
	// Checks an HRESULT/StatusCode and if it's not S_OK it stores to the log and returns the HRESULT converted to StatusCode. (thread safe)
	// This is a macro, so it can be used to return out of a function on failure
#define LogCheckWithReturn(cond,cat,ret) {HRESULT hr = cond; if(hr != S_OK) { auto scode = (StatusCode)hr; Log::_record(string(#cond " failed with code ") + StatusCodeToString(scode) ,cat,__LINE__,__func__,__FILE__); return scode; }}
	// Checks an assert and if false stores to the log and triggers a debug break (thread safe)
#define LogAssertWithBreak(cond,cat) if(!cond) { Log::_record(#cond L" != true",cat,__LINE__,__func__,__FILE__); DebugBreak();  }
	// Checks an assert, stores to the log if false and returns the condition. Can be used inside an if (thread safe)
#define LogAssertAndContinue(cond,cat) [](){ if(cond) Log::_record(#cond L" != true",cat,__LINE__,__func__,__FILE__); return cond; }() 

	class Log
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

		static void _record(const wstring& Message,LogCategory Category,int Line,const wstring& Function,const wstring& File)
		{
			Entry e;
			e.Category = Category;
			e.Message = Message;
			e.Timestamp = chrono::system_clock::now();
			e.Line = Line;
			e.Function = Function;
			e.File = File;

			Records.push_back(e);
		}

		// Prints the entire log to the supplied stream.
		// The stream can be a file, cout, or any other ostream.
		static void PrintAll(wostream& OutputStream)
		{
			for(const auto& e : Records)
			{
				auto time = chrono::system_clock::to_time_t(e.Timestamp);
				tm timeinfo;
				localtime_s(&timeinfo,&time);
				OutputStream << L"[" << put_time(&timeinfo, L"%T") << L"] " << cat_name << L" : " << e.Message << endl;
				OutputStream <<	L"    on line " << e.Line << L" of file " << e.File << L", function " << e.Function << endl;
			}
		}

		// Equal to PrintAll, but only prints a range of logs instead of all of them
		static void PrintRange(size_t Start, size_t End, wostream& OutputStream)
		{
			for(size_t i = Start; i < End && i < Records.size();i++)
			{
				const auto& e = Records[i];
				auto time = chrono::system_clock::to_time_t(e.Timestamp);
				tm timeinfo;
				localtime_s(&timeinfo,&time);
				OutputStream << L"[" << put_time(&timeinfo, L"%T") << L"] " << cat_name << L" : " << e.Message << endl;
				OutputStream <<	L"    on line " << e.Line << L" of file " << e.File << L", function " << e.Function << endl;
			}
		}
	private:
		static concurrency::concurrent_vector<Entry> Records;
		static const wchar_t* cat_name[(int)LogCategory::_LogCategoryCount] = { L"Info", L"Warning", L"Error", L"CriticalError" };
	};
}