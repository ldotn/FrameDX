#pragma once
#include "stdafx.h"

namespace FrameDX
{
	enum class StatusCode 
	{ 
		Ok                    = S_OK,
		Aborted               = E_ABORT,
		AccessDenied          = E_ACCESSDENIED,
		Failed                = E_FAIL,
		InvalidHandle         = E_HANDLE,
		InvalidArgument       = E_INVALIDARG,
		InterfaceNotSupported = E_NOINTERFACE,
		NotImplemented        = E_NOTIMPL,
		OutOfMemory           = E_OUTOFMEMORY,
		InvalidPointer        = E_POINTER
	};
	
	wstring StatusCodeToString(StatusCode code)
	{
		switch(code)
		{
		case StatusCode::Ok:
			return L"Ok";
		case StatusCode::Aborted:
			return L"Aborted";
		case StatusCode::AccessDenied:
			return L"Acess Denied";
		case StatusCode::Failed:
			return L"Failed";
		case StatusCode::InvalidHandle:
			return L"Invalid Handle";
		case StatusCode::InvalidArgument:
			return L"Invalid Argument";
		case StatusCode::InterfaceNotSupported:
			return L"Interface Not Supported";
		case StatusCode::NotImplemented:
			return L"Not Implemented";
		case StatusCode::OutOfMemory:
			return L"Out Of Memory";
		case StatusCode::InvalidPointer:
			return L"Invalid Pointer";
		default:
			return L"Unknown Status Code";
		}
	}

#define LAST_ERROR (StatusCode)HRESULT_FROM_WIN32(GetLastError())

	enum class KeyAction { Up, Down };
}