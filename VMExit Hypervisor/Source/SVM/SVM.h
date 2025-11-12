#pragma once

#include <ntifs.h>

class SVMUtilities
{
public:
	static bool IsSvmEnabled();

	static VOID EnableSvmOnCore();
};