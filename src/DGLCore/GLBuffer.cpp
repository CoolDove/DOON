#include "GLBuffer.h"
#include "DoveLog.hpp"

namespace DGL {

Buffer::Buffer() {
	glCreateBuffers(1, &m_id);
	m_size = m_capacity = -1;
}
Buffer::~Buffer() {
	glDeleteBuffers(1, &m_id);
}
void Buffer::allocate(size_t _size, BufferFlag _flag) {
	glNamedBufferStorage(m_id, 2 * _size, nullptr, static_cast<unsigned int>(_flag));
	m_flag = static_cast<unsigned int>(_flag);
	m_size = _size;
	m_capacity = 2 * _size;
}

void Buffer::upload_data(size_t _size, size_t _offset, void* _data) {
	if (_size + _offset > m_capacity) {
		GLuint temp_buffer;
		glCreateBuffers(1, &temp_buffer);
		// allocate a new bigger buffer
		glNamedBufferStorage(temp_buffer, 2 * (_size + _offset), nullptr, m_flag); 
		// copy the current data to it
		glCopyNamedBufferSubData(m_id, temp_buffer, 0, 0, m_size);
		glBufferSubData(temp_buffer, _offset, _size, _data);

		glNamedBufferStorage(m_id, 2 * (_size + _offset), nullptr, m_flag); 
		glCopyNamedBufferSubData(temp_buffer, m_id, 0, 0, m_size);

		glDeleteBuffers(1, &temp_buffer);

		m_size = _offset + _size;
		m_capacity = 2 * (_size + _offset);
	} else {
		glNamedBufferSubData(m_id, _offset, _size, _data);
		m_size = (_offset + _size) > m_size ? (_offset + _size) : m_size;
	}
}
void Buffer::bind(BufferType _target) {
	glBindBuffer(static_cast<unsigned int>(_target), m_id);
	m_type = _target;
}
}