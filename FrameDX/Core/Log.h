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
#define LogAssert(cond,cat) if(!cond) Log::_record(#cond " != true",cat,__LINE__,__func__,__FILE__)
	// Checks an assert and if false stores to the log and returns the "ret" value. (thread safe)
	// This is a macro, so it can be used to return out of a function on failure
#define LogAssertWithReturn(cond,cat,ret) if(!cond) { Log::_record(#cond " != true",cat,__LINE__,__func__,__FILE__); return ret; }
	// Checks an assert and if false stores to the log and triggers a debug break (thread safe)
#define LogAssertWithBreak(cond,cat) if(!cond) { Log::_record(#cond " != true",cat,__LINE__,__func__,__FILE__); DebugBreak();  }

	class Log
	{
	public:
		struct Entry
		{
			LogCategory Category;
			string Message;

			chrono::system_clock::time_point Timestamp;
			int Line;
			string Function;
			string File;
		};

		static void _record(const string& Message,LogCategory Category,int Line,const string& Function,const string& File)
		{
			Entry e;
			e.Category = Category;
			e.Message = Message;
			e.Timestamp = chrono::system_clock::now();
			e.Line = Line;
			e.Function = Function;
			e.File = File;

			LogQueue.push_back(e);
		}

		// Prints the entire log to the supplied stream.
		// The stream can be a file, cout, or any other ostream.
		// This function locks the entire log, so no writes are allowed
		static void PrintAll(ostream& OutputStream)
		{
			static const char* cat_name[(int)LogCategory::_LogCategoryCount] = { "Info", "Warning", "Error", "CriticalError" };
			for(const auto& e : LogQueue)
			{
				auto time = chrono::system_clock::to_time_t(e.Timestamp);
				tm timeinfo;
				localtime_s(&timeinfo,&time);
				OutputStream << "[" << put_time(&timeinfo, "%T") << "] " << cat_name << " : " << e.Message << endl;
				OutputStream <<	"    on line " << e.Line << " of file " << e.File << ", function " << e.Function << endl;
			}
		}
	private:
		static concurrency::concurrent_vector<Entry> LogQueue;
	};
}