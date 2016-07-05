#pragma once
#include "EagleLib/detail/ParameteredIObjectImpl.hpp"

#ifdef _MSC_VER
#define BEGIN_PARAMS(...) BOOST_PP_CAT( BOOST_PP_OVERLOAD(BEGIN_PARAMS__, __VA_ARGS__ )(__VA_ARGS__, __COUNTER__), BOOST_PP_EMPTY() )
#else
#define BEGIN_PARAMS(...) BOOST_PP_OVERLOAD(BEGIN_PARAMS__, __VA_ARGS__ )(__VA_ARGS__, __COUNTER__)
#endif


#define RANGED_PARAM(type, name, init, min, max) RANGED_PARAM_(type, name, init, min, max, __COUNTER__);

#define END_PARAMS END_PARAMS__(__COUNTER__);