#include <windows.h>
#include <iostream>

#include "microtest.h"
#include "VMemMirrorBuffer.h"
#include "RingBuffer.h"

#include <sysinfoapi.h>

TEST(PRINT_SYSTEM_INFO) {
	SYSTEM_INFO sys{};
	GetSystemInfo(&sys);

	std::cout << "**************** System Info ****************" << std::endl;
	std::cout << "PageSize: " << sys.dwPageSize << std::endl;
	std::cout << "*********************************************" << std::endl;
}

TEST(CHAR_BUFFER_REPORTS_DATA_STATE_CORRECTLY) {
	RingBuffer<char> b{ 4 };

	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());

	b.write('a');
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	b.write('b');
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	b.write('c');
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());

	b.write('d');
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());

	b.read();
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	b.read();
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	b.read();
	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());

	b.read();
	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());

	b.write('e');
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

}

TEST(CHAR_BUFFER_RW_CORRECTLY_FILL_EMPTY) {
	RingBuffer<char> b{ 4 };

	b.write('a');
	b.write('b');
	b.write('c');

	char r;
	r = b.read();
	ASSERT_EQ('a', r);
	r = b.read();
	ASSERT_EQ('b', r);
	r = b.read();
	ASSERT_EQ('c', r);
}

TEST(CHAR_BUFFER_RW_CORRECTLY_LOCKSTEP) {
	RingBuffer<char> b{ 4 };
	char r;

	b.write('a');
	r = b.read();
	ASSERT_EQ('a', r);

	b.write('b');
	r = b.read();
	ASSERT_EQ('b', r);
	
	b.write('c');
	r = b.read();
	ASSERT_EQ('c', r);
}

TEST(CHAR_BUFFER_RW_CORRECTLY_LOCKSTEP_OVERFLOW) {
	RingBuffer<char> b{ 4 };
	char r;

	b.write('a');
	r = b.read();
	ASSERT_EQ('a', r);

	b.write('b');
	r = b.read();
	ASSERT_EQ('b', r);

	b.write('c');
	r = b.read();
	ASSERT_EQ('c', r);

	b.write('d');
	r = b.read();
	ASSERT_EQ('d', r);

	b.write('e');
	r = b.read();
	ASSERT_EQ('e', r);
}


TEST(CHAR_BUFFER_RW_CORRECTLY_FILL_EMPTY_OVERFLOW) {
	RingBuffer<char> b{ 4 };

	b.write('a');
	b.write('b');
	b.write('c');
	b.write('d');
	b.write('e');

	char r;
	r = b.read();
	ASSERT_EQ('c', r);
	r = b.read();
	ASSERT_EQ('d', r);
	r = b.read();
	ASSERT_EQ('e', r);
}

TEST(CHAR_BUFFER_COMPLEX_RW) {
	RingBuffer<char> b{ 4 };
	char r;

	b.write('a');
	b.write('b');
	b.write('c');
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());
	
	r = b.read();
	ASSERT_EQ('a', r);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());
	
	b.write('d');
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());
	
	b.write('e');
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());

	r = b.read();
	ASSERT_EQ('c', r);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	r = b.read();
	ASSERT_EQ('d', r);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	r = b.read();
	ASSERT_EQ('e', r);
	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());

	r = b.read();
	ASSERT_EQ((char)0, r);
	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());
}

TEST(TEST_MIRROR_BUFFER) {
	VMemMirrorBuffer buffer{};
	size_t pageSize = System::getPageSize();
	
	buffer.allocate(pageSize);

	ASSERT_TRUE(buffer.isAllocated())

	char* ptr = static_cast<char*>(buffer.getRawBuffer());

	for (size_t i = 0; i < pageSize; i++) {
		ptr[i] = static_cast<char>(i % 128);
	}

	for (size_t i = 0; i < pageSize; i++) {
		ASSERT_EQ(ptr[i + pageSize], static_cast<char>(i % 128));
	}

	for (size_t i = 0; i < pageSize; i++) {
		ptr[i] = static_cast<char>(0);
	}

	for (size_t i = 0; i < pageSize; i++) {
		ASSERT_EQ(ptr[i + pageSize], static_cast<char>(0));
	}

	for (size_t i = 0; i < pageSize; i++) {
		ptr[i + pageSize] = static_cast<char>(i % 128);
	}

	for (size_t i = 0; i < pageSize; i++) {
		ASSERT_EQ(ptr[i], static_cast<char>(i % 128));
	}
}

TEST_MAIN();