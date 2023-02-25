#include "microtest.h"
#include "RingBuffer.h"

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

TEST_MAIN();