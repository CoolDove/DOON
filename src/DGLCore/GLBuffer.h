#pragma once
#include <glad/glad.h>
#include "BitMaskEnum.h"

namespace DGL
{
enum class BufferFlag : unsigned int {
	DYNAMIC_STORAGE_BIT = 0x0100,
	MAP_READ_BIT 				= 0x0001,
	MAP_WRITE_BIT 			= 0x0002,
	MAP_PERSISTENT_BIT 	= 0x0040,
	MAP_COHERENT_BIT 		= 0x0080,
	CLIENT_STORAGE_BIT 	= 0x0200
};
BIT_MASK_ENUM(BufferFlag)

enum class BufferType : unsigned int {
	VERTEX_BUFFER 				= 0x8892,			// GL_ARRAY_BUFFER 					0x8892
	COPY_READ_BUFFER 			= 0x8F36,     // GL_COPY_READ_BUFFER 			0x8F36
	COPY_WRITE_BUFFER 		= 0x8F37,     // GL_COPY_WRITE_BUFFER 		0x8F37
	DRAW_INDIRECT_BUFFER 	= 0x8F3F,     // GL_DRAW_INDIRECT_BUFFER 	0x8F3F
	ELEMENT_INDEX_BUFFER 	= 0x8893,     // GL_ELEMENT_ARRAY_BUFFER 	0x8893
	PIXEL_PACK_BUFFER 		= 0x88EB,     // GL_PIXEL_PACK_BUFFER 		0x88EB
	PIXEL_UNPACK_BUFFER 	= 0x88EC,     // GL_PIXEL_UNPACK_BUFFER 	0x88EC
	TEXTURE_BUFFER 				= 0x8C2A,     // GL_TEXTURE_BUFFER 				0x8C2A
	UNIFORM_BUFFER 				= 0x8A11,     // GL_UNIFORM_BUFFER 				0x8A11
//here should be a TRANSFORM_FEEDBACK_BUFFER
};

class Buffer {
public:
	void allocate(size_t _size, BufferFlag _flag);
	// void allocate_init(size_t _size, void* _data, BufferFlag _flag);
	void upload_data(size_t _size, size_t _offset, void* _data);
	void bind(BufferType _target);
public:
	Buffer();
	~Buffer();
	int get_id() { return m_id; }
	size_t get_size() { return m_size; }
	size_t get_capacity() { return m_capacity; }
	BufferType get_type() { return m_type; }
private:
	GLuint m_id;
	size_t m_size;
	size_t m_capacity;
	BufferType m_type;
	unsigned int m_flag; 
};
}