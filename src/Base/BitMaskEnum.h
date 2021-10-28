#pragma once
#define BIT_MASK_ENUM(T) \
inline T operator |(T _lhs, T _rhs)\
	{return static_cast<T>(static_cast<unsigned int>(_lhs) | static_cast<unsigned int>(_rhs));}\
inline T operator &(T _lhs, T _rhs)\
	{return static_cast<T>(static_cast<unsigned int>(_lhs) & static_cast<unsigned int>(_rhs));}\
inline T operator ^(T _lhs, T _rhs)\
	{return static_cast<T>(static_cast<unsigned int>(_lhs) ^ static_cast<unsigned int>(_rhs));}\
inline T operator ~(T _rhs)\
	{return static_cast<T>(~static_cast<unsigned int>(_rhs));}\
inline T& operator |=(T& _lhs, T _rhs)\
	{unsigned int tmp = static_cast<unsigned int>(_lhs);\
	 tmp |= static_cast<unsigned int>(_rhs);_lhs = static_cast<T>(tmp);\
	 return _lhs;}\
inline T operator &=(T& _lhs, T _rhs)\
	{unsigned int tmp = static_cast<unsigned int>(_lhs);\
	 tmp &= static_cast<unsigned int>(_rhs);_lhs = static_cast<T>(tmp);\
	 return _lhs;}\
inline T operator ^=(T& _lhs, T _rhs)\
	{unsigned int tmp = static_cast<unsigned int>(_lhs);\
	 tmp ^= static_cast<unsigned int>(_rhs);_lhs = static_cast<T>(tmp);\
	 return _lhs;}\
inline unsigned int get_uint(const T& _lhs) {\
	return static_cast<unsigned int>(_lhs);}
