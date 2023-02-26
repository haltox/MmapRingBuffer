#pragma once

#include <type_traits>
#include <Memoryapi.h>
#include <WinBase.h>

//TODO pad T to n^2 size?
// 

template <typename T>
class RingBuffer {
	static_assert(std::is_trivial<T>::value, "RingBuffer must be templated on a trivial type.");

private:
	RingBuffer() {}

public:
	
	RingBuffer(size_t nbBuckets);
	RingBuffer(const RingBuffer &rhs);
	RingBuffer(RingBuffer &&rhs);
	~RingBuffer();
	RingBuffer& operator=(RingBuffer&& rhs);
	RingBuffer& operator=(const RingBuffer& rhs);

	bool hasData() const;
	bool isFull() const;

	// T is returned as an R value since the bucket is considered as 
	// freed after the operation. Caller should copy or move but not keep
	// the ref.
	T&& read();

	// T is taken as a const ref to support rvalue refs while maintaining
	// a non destructive behaviour on the input.
	void write(const T& t);

private:
	size_t inc(size_t base) const;

private:
	// Number of buckets available for data type T - not the size in bytes
	// of the buffer.
	// Typically, nbBuckets - 1 will be available for use at once. 
	size_t _nbBuckets {0};

	// The buffer of the ring buffer
	T* _buffer {nullptr};

	// Read and write heads
	size_t _read {0};
	size_t _write {0};
};

template <typename T>
RingBuffer<T>::RingBuffer(size_t nbBuckets)
	: _nbBuckets {nbBuckets}
{
	_buffer = new T[nbBuckets];
}

template <typename T>
RingBuffer<T>::RingBuffer(const RingBuffer<T>& rhs)
{
	this();
	*this = rhs;
}

template <typename T>
RingBuffer<T>::RingBuffer(RingBuffer<T>&& rhs)
{
	this();
	*this = std::move(rhs);
	std::swap(*this, rhs);
}

template <typename T>
RingBuffer<T>::~RingBuffer() {
	delete[] _buffer;
}

template <typename T>
RingBuffer<T>& RingBuffer<T>::operator=(RingBuffer<T>&& rhs)
{
	*this = std::move(rhs);
	return *this;
}

template <typename T>
RingBuffer<T>& RingBuffer<T>::operator=(const RingBuffer<T>& rhs)
{
	delete[] _buffer;
	_nbBuckets = rhs._nbBuckets;

	// since is_trivial<T> == true could be replaced by memcpy 
	T* l; T* r;
	for (l = _buffer, r = rhs.buffer; 
		 l <= rhs._buffer + rhs.nbBuckets; 
		 l++, r++) {
		*l = *r;
	}

	_buffer = new T[rhs._nbBuckets];
}

template <typename T>
bool RingBuffer<T>::hasData() const
{
	return _read != _write;
}

template <typename T>
bool RingBuffer<T>::isFull() const
{
	return inc(_write) == _read;
}

template <typename T>
T&& RingBuffer<T>::read()
{
	if (!hasData()) 
	{
		return std::move(T{});
	}

	size_t i = _read;
	_read = inc(_read);

	return std::move(_buffer[i]);
}

template <typename T>
void RingBuffer<T>::write(const T& t)
{
	_buffer[_write] = t;
	_write = inc(_write);
	_read = (_write == _read) ? inc(_read) : _read;
}

template <typename T>
size_t RingBuffer<T>::inc(size_t base) const
{
	return (base + 1) % _nbBuckets;
}
