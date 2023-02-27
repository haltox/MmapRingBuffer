#include <windows.h>
#include <iostream>

#include "microtest.h"
#include "VMemMirrorBuffer.h"
#include "RingBuffer.h"

#include <sysinfoapi.h>

struct BUFFED_CHAR {
	char v;
	char buff[1023];
};

TEST(PRINT_SYSTEM_INFO) {
	SYSTEM_INFO sys{};
	GetSystemInfo(&sys);

	std::cout << "**************** System Info ****************" << std::endl;
	std::cout << "PageSize: " << sys.dwPageSize << std::endl;
	std::cout << "*********************************************" << std::endl;
}

TEST(TEST_CONSTANTS) {
	ASSERT_EQ(1024, sizeof(BUFFED_CHAR));
}

TEST(CHAR_BUFFER_REPORTS_DATA_STATE_CORRECTLY) {
	RingBuffer<BUFFED_CHAR> b{ 4 };

	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());

	BUFFED_CHAR c;
	c.v = 'a';
	b.write(c);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	c.v = 'b';
	b.write(c);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	c.v = 'c';
	b.write(c);
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());

	c.v = 'd';
	b.write(c);
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

	c.v = 'e';
	b.write(c);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

}

TEST(CHAR_BUFFER_RW_CORRECTLY_FILL_EMPTY) {
	RingBuffer<BUFFED_CHAR> b{ 4 };

	BUFFED_CHAR cw;

	cw.v = 'a';
	b.write(cw);
	cw.v = 'b';
	b.write(cw);
	cw.v = 'c';
	b.write(cw);

	BUFFED_CHAR cr;
	cr = b.read();
	ASSERT_EQ('a', cr.v);
	cr = b.read();
	ASSERT_EQ('b', cr.v);
	cr = b.read();
	ASSERT_EQ('c', cr.v);
}

TEST(CHAR_BUFFER_RW_CORRECTLY_LOCKSTEP) {
	RingBuffer<BUFFED_CHAR> b{ 4 };

	BUFFED_CHAR cr;
	BUFFED_CHAR cw;

	cw.v = 'a';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('a', cr.v);

	cw.v = 'b';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('b', cr.v);
	
	cw.v = 'c';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('c', cr.v);
}

TEST(CHAR_BUFFER_RW_CORRECTLY_LOCKSTEP_OVERFLOW) {
	RingBuffer<BUFFED_CHAR> b{ 4 };

	BUFFED_CHAR cr;
	BUFFED_CHAR cw;

	cw.v = 'a';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('a', cr.v);

	cw.v = 'b';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('b', cr.v);

	cw.v = 'c';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('c', cr.v);

	cw.v = 'd';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('d', cr.v);

	cw.v = 'e';
	b.write(cw);
	cr = b.read();
	ASSERT_EQ('e', cr.v);
}


TEST(CHAR_BUFFER_RW_CORRECTLY_FILL_EMPTY_OVERFLOW) {
	RingBuffer<BUFFED_CHAR> b{ 4 };

	BUFFED_CHAR cr;
	BUFFED_CHAR cw;

	cw.v = 'a';
	b.write(cw);
	cw.v = 'b';
	b.write(cw);
	cw.v = 'c';
	b.write(cw);
	cw.v = 'd';
	b.write(cw);
	cw.v = 'e';
	b.write(cw);

	cr = b.read();
	ASSERT_EQ('c', cr.v);
	cr = b.read();
	ASSERT_EQ('d', cr.v);
	cr = b.read();
	ASSERT_EQ('e', cr.v);
}

TEST(CHAR_BUFFER_COMPLEX_RW) {
	RingBuffer<BUFFED_CHAR> b{ 4 };

	BUFFED_CHAR cr;
	BUFFED_CHAR cw;

	cw.v = 'a';
	b.write(cw);
	cw.v = 'b';
	b.write(cw);
	cw.v = 'c';
	b.write(cw);
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());
	
	cr = b.read();
	ASSERT_EQ('a', cr.v);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());
	
	cw.v = 'd';
	b.write(cw);
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());
	
	cw.v = 'e';
	b.write(cw);
	ASSERT_TRUE(b.hasData());
	ASSERT_TRUE(b.isFull());

	cr = b.read();
	ASSERT_EQ('c', cr.v);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	cr = b.read();
	ASSERT_EQ('d', cr.v);
	ASSERT_TRUE(b.hasData());
	ASSERT_FALSE(b.isFull());

	cr = b.read();
	ASSERT_EQ('e', cr.v);
	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());

	cr = b.read();
	//ASSERT_EQ(0, cr.v); // don't assert value - it might be garbo
	ASSERT_FALSE(b.hasData());
	ASSERT_FALSE(b.isFull());
}

TEST(TEST_BUCKETS_COUNT) {
	RingBuffer<BUFFED_CHAR> b{ 4 };
	BUFFED_CHAR cr;
	cr.v = 'a';

	ASSERT_EQ(b.availableBuckets(), 3);
	ASSERT_EQ(b.availableForRead(), 0);
	ASSERT_FALSE(b.isFull());
	ASSERT_FALSE(b.hasData());
	
	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 2);
	ASSERT_EQ(b.availableForRead(), 1);
	ASSERT_FALSE(b.isFull());

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 1);
	ASSERT_EQ(b.availableForRead(), 2);
	ASSERT_FALSE(b.isFull());

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());

	b.read();
	ASSERT_EQ(b.availableBuckets(), 1);
	ASSERT_EQ(b.availableForRead(), 2);

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());

	b.read();
	ASSERT_EQ(b.availableBuckets(), 1);
	ASSERT_EQ(b.availableForRead(), 2);

	b.read();
	ASSERT_EQ(b.availableBuckets(), 2);
	ASSERT_EQ(b.availableForRead(), 1);

	b.read();
	ASSERT_EQ(b.availableBuckets(), 3);
	ASSERT_EQ(b.availableForRead(), 0);
	ASSERT_FALSE(b.hasData());

	b.read();
	ASSERT_EQ(b.availableBuckets(), 3);
	ASSERT_EQ(b.availableForRead(), 0);
	ASSERT_FALSE(b.hasData());

	b.read();
	ASSERT_EQ(b.availableBuckets(), 3);
	ASSERT_EQ(b.availableForRead(), 0);
	ASSERT_FALSE(b.hasData());

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 2);
	ASSERT_EQ(b.availableForRead(), 1);

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 1);
	ASSERT_EQ(b.availableForRead(), 2);

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());

	b.read();
	ASSERT_EQ(b.availableBuckets(), 1);
	ASSERT_EQ(b.availableForRead(), 2);

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());

	b.read();
	ASSERT_EQ(b.availableBuckets(), 1);
	ASSERT_EQ(b.availableForRead(), 2);

	b.read();
	ASSERT_EQ(b.availableBuckets(), 2);
	ASSERT_EQ(b.availableForRead(), 1);

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 1);
	ASSERT_EQ(b.availableForRead(), 2);

	b.write(cr);
	ASSERT_EQ(b.availableBuckets(), 0);
	ASSERT_EQ(b.availableForRead(), 3);
	ASSERT_TRUE(b.isFull());
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