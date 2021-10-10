#include "GLBuffer.h"
#include "DoveLog.hpp"

namespace DGL {

Buffer::Buffer() {
	glCreateBuffers(1, &id_);
	size_ = capacity_ = -1;
}
Buffer::~Buffer() {
	glDeleteBuffers(1, &id_);
}
void Buffer::allocate(size_t _size, BufferFlag _flag) {
	glNamedBufferStorage(id_, 2 * _size, nullptr, static_cast<unsigned int>(_flag));
	flag_ = static_cast<unsigned int>(_flag);
	size_ = _size;
	capacity_ = 2 * _size;
}

void Buffer::upload_data(size_t _size, size_t _offset, void* _data) {
	if (_size + _offset > capacity_) {
		GLuint temp_buffer;
		glCreateBuffers(1, &temp_buffer);
		// allocate a new bigger buffer
		glNamedBufferStorage(temp_buffer, 2 * (_size + _offset), nullptr, flag_); 
		// copy the current data to it
		glCopyNamedBufferSubData(id_, temp_buffer, 0, 0, size_);
		glBufferSubData(temp_buffer, _offset, _size, _data);

		glNamedBufferStorage(id_, 2 * (_size + _offset), nullptr, flag_); 
		glCopyNamedBufferSubData(temp_buffer, id_, 0, 0, size_);

		glDeleteBuffers(1, &temp_buffer);

		size_ = _offset + _size;
		capacity_ = 2 * (_size + _offset);
	} else {
		glNamedBufferSubData(id_, _offset, _size, _data);
		size_ = (_offset + _size) > size_ ? (_offset + _size) : size_;
	}
}
void Buffer::bind(BufferType _target) {
	glBindBuffer(static_cast<unsigned int>(_target), id_);
	type_ = _target;
}
}