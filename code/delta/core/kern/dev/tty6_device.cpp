
/*
 * PS4Delta : PS4 emulation and research project
 *
 * Copyright 2019-2020 Force67.
 * For information regarding licensing see LICENSE
 * in the root of the source tree.
 */

#include "tty6_device.h"

namespace krnl
{
	tty6Device::tty6Device(proc* p) :
		device(p)
	{}

	bool tty6Device::init(const char*, uint32_t, uint32_t)
	{
		return true;
	}

	uint8_t* tty6Device::map(void* addr, size_t, uint32_t, uint32_t, size_t)
	{
		__debugbreak();
		return reinterpret_cast<uint8_t*>(-1);
		//return SysError::SUCCESS;
	}
}