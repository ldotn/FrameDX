#include "stdafx.h"
#include "Log.h"

void FrameDX::__log::_record(const wstring & Message, LogCategory Category, int Line, const wstring & Function, const wstring & File)
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

void FrameDX::__log::PrintAll(wostream & OutputStream)
{
	for(const auto& e : Records)
	{
		auto time = chrono::system_clock::to_time_t(e.Timestamp);
		tm timeinfo;
		localtime_s(&timeinfo, &time);
		OutputStream << L"[" << put_time(&timeinfo, L"%T") << L"] " << cat_name[(int)e.Category] << L" : " << e.Message << endl;
		OutputStream << L"    on line " << e.Line << L" of file " << e.File << L", function " << e.Function << endl;
	}

}

void FrameDX::__log::PrintRange(size_t Start, size_t End, wostream & OutputStream)
{
	for(size_t i = Start; i < End && i < Records.size(); i++)
	{
		const auto& e = Records[i];
		auto time = chrono::system_clock::to_time_t(e.Timestamp);
		tm timeinfo;
		localtime_s(&timeinfo, &time);
		OutputStream << L"[" << put_time(&timeinfo, L"%T") << L"] " << cat_name[(int)e.Category] << L" : " << e.Message << endl;
		OutputStream << L"    on line " << e.Line << L" of file " << e.File << L", function " << e.Function << endl;
	}
}
