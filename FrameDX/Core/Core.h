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

#define LAST_ERROR (StatusCode)HRESULT_FROM_WIN32(GetLastError())

	enum class KeyAction { Up, Down };
}