#pragma once

#include <WinBase.h>
#include <cstdint>
#include <stdexcept>

#include "System.h"

// This one is hard to name XD
// Impl of a large buffer segmented in 2 equal size parts each mapping to
// the same paging file.
// Overflow past the mid point of the large buffer actually loop back to the first half.
//
class VMemMirrorBuffer {

public:
	VMemMirrorBuffer() {}
	VMemMirrorBuffer(size_t size);
	~VMemMirrorBuffer();

	bool allocate(size_t size);
	void free();

	bool isAllocated() const {
		return _allocated;
	}

	void* getRawBuffer() {
		return _actualBuffer;
	}

	size_t getPageSize() const { return _size; }
	size_t getVMemSize() const { return _size * 2; }

private:


private:
	bool _allocated {false};
	size_t _size {0};
	void* _actualBuffer{ nullptr };

	// Stuff for resource mgmt
	HANDLE _pageFile{ INVALID_HANDLE_VALUE };
	void* _view1{ nullptr };
	void* _view2{ nullptr };

	void* _firstSegment{ nullptr };
	void* _secondSegment{ nullptr };

};

VMemMirrorBuffer::VMemMirrorBuffer(size_t size)
{

	allocate(size);
}

VMemMirrorBuffer::~VMemMirrorBuffer()
{
	free();
}

bool VMemMirrorBuffer::allocate(size_t size)
{
	if (size % System::getPageSize() != 0) {
		throw std::runtime_error{ "VMemMirrorBuffer alloc size must be a multiple of System::pageSize" };
	}

	free();
	_size = size;

	try {

		// Reserve block 2 x size
		_actualBuffer = VirtualAlloc2(
			nullptr, // Same process
			nullptr, // No starting address, get a new block
			2 * _size,
			MEM_RESERVE | MEM_RESERVE_PLACEHOLDER,
			PAGE_NOACCESS,
			nullptr, 0 // No additional parameters
		);

		if (_actualBuffer == nullptr) {
			throw std::runtime_error{"couldn't alloc buffer"};
		}

		// Segment block in 2 - after virtual free, we have 2 distinct placeholder regions.
		if (!VirtualFree(_actualBuffer, _size, MEM_RELEASE | MEM_PRESERVE_PLACEHOLDER))
		{
			throw std::runtime_error{ "couldn't segment buffer" };
		}

		_firstSegment = _actualBuffer;
		_secondSegment = (char*)_actualBuffer + _size;

		// step 3 - create page mapping section

		uint32_t lowBitsSize = static_cast<uint32_t>(0xFFFFFFFF & _size);
		uint32_t highBitsSize = static_cast<uint32_t>(0xFFFFFFFF & (_size >> 32));

		_pageFile = CreateFileMapping(
			INVALID_HANDLE_VALUE,	// Create file mapping backed by a paging file
			nullptr,				// no inherit
			PAGE_READWRITE,			// rw access
			highBitsSize,			// high order bytes of size
			lowBitsSize,			// Low-order bytes of size
			nullptr					// anonymous region
		);

		if (_pageFile == NULL) {
			throw std::runtime_error{ "couldn't allocate file mapping" };
		}

		// Step 4 map segments to page
		_view1 = (char*)MapViewOfFile3(
			_pageFile,
			nullptr,
			_firstSegment,
			highBitsSize,
			lowBitsSize,
			MEM_REPLACE_PLACEHOLDER,
			PAGE_READWRITE,
			nullptr, 0
		);

		_view2 = (char*)MapViewOfFile3(
			_pageFile,
			nullptr,
			_secondSegment,
			highBitsSize,
			lowBitsSize,
			MEM_REPLACE_PLACEHOLDER,
			PAGE_READWRITE,
			nullptr, 0
		);

		if (_view1 == nullptr)
		{
			throw std::runtime_error{ "view1 mapping failed" };
		}

		if (_view2 == nullptr)
		{
			throw std::runtime_error{ "view2 mapping failed" };
		}

		_allocated = true;

		return true;
	}
	catch( std::runtime_error& ex ) 
	{
		free();


		throw ex;
	}
}

void VMemMirrorBuffer::free()
{
	if (_view1 != nullptr) {
		UnmapViewOfFileEx(_view1, 0);
	}

	if (_view2 != nullptr) {
		UnmapViewOfFileEx(_view2, 0);
	}

	if (_pageFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_pageFile);
	}

	if (_secondSegment != nullptr) 
	{
		VirtualFree(_secondSegment, 0, MEM_RELEASE);
		_secondSegment = nullptr;
	}

	if (_actualBuffer != nullptr) {
		VirtualFree(_actualBuffer, 0, MEM_RELEASE);
		_actualBuffer = nullptr;
		_firstSegment = nullptr;
	}

	_allocated = false;
}
