#pragma once
#include "stdafx.h"

namespace FrameDX
{
	enum class StatusCode 
	{ 
		Ok                        = S_OK,
		Aborted                   = E_ABORT,
		AccessDenied              = E_ACCESSDENIED,
		Failed                    = E_FAIL,
		InvalidHandle             = E_HANDLE,
		InvalidArgument           = E_INVALIDARG,
		InterfaceNotSupported     = E_NOINTERFACE,
		NotImplemented            = E_NOTIMPL,
		OutOfMemory               = E_OUTOFMEMORY,
		InvalidPointer            = E_POINTER,
		FileNotFound              = D3D11_ERROR_FILE_NOT_FOUND,
		TooManyUniqueStateObjects = D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS,
		TooManyUniqueViewObjects  = D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS,
		MapWithoutInitialDiscard  = D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD,
		InvalidCall               = DXGI_ERROR_INVALID_CALL,
		StillDrawing              = DXGI_ERROR_WAS_STILL_DRAWING
	};
	
	inline wstring StatusCodeToString(StatusCode code)
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
		case StatusCode::FileNotFound:
			return L"File Not Found";
		case StatusCode::TooManyUniqueStateObjects:
			return L"Too Many Unique State Objects";
		case StatusCode::TooManyUniqueViewObjects:
			return L"Too Many Unique View Objects";
		case StatusCode::MapWithoutInitialDiscard:
			return L"Map Without Initial Discard";
		case StatusCode::InvalidCall:
			return L"Invalid Call";
		case StatusCode::StillDrawing:
			return L"StillDrawing";
		default:
			return L"Unknown Status Code";
		}
	}

#define LAST_ERROR (StatusCode)HRESULT_FROM_WIN32(GetLastError())

	enum class KeyAction { Up, Down };
}