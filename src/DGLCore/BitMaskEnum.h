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
inline T operator |=(T& _lhs, T _rhs)\
	{unsigned int tmp = static_cast<unsigned int>(_lhs);\
	 tmp |= static_cast<unsigned int>(_rhs);\
	 return static_cast<T>(tmp);}\
inline T operator &=(T& _lhs, T _rhs)\
	{unsigned int tmp = static_cast<unsigned int>(_lhs);\
	 tmp &= static_cast<unsigned int>(_rhs);\
	 return static_cast<T>(tmp);}\
inline T operator ^=(T& _lhs, T _rhs)\
	{unsigned int tmp = static_cast<unsigned int>(_lhs);\
	 tmp ^= static_cast<unsigned int>(_rhs);\
	 return static_cast<T>(tmp);}