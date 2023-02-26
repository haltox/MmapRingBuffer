#pragma once

#include <windows.h>

namespace System {
	size_t getPageSize()
	{
		static size_t pageSize = 0;

		if (pageSize == 0)
		{
			SYSTEM_INFO sys{};
			GetSystemInfo(&sys);
			pageSize = sys.dwPageSize;
		}

		return pageSize;
	}
};