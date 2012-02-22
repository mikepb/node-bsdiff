#ifndef NODE_BSDIFF_ENDIAN_H
#define NODE_BSDIFF_ENDIAN_H

#include "boost/endian.hpp"

#ifdef BOOST_LITTLE_ENDIAN
# define b32le(x) (x)
#else
# include "gnuclib/byteswap.h"
# define b32le(x) bswap_32(x)
#endif

#endif // NODE_BSDIFF_ENDIAN_H
