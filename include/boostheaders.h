#pragma once

//  Configuring boost
//  We will define boost options here rather in boost/config/user.hpp

// Tells the config system not to automatically select 
// which libraries to link against.
#define BOOST_ALL_NO_LIB

#ifdef WINCE
//  TODO: Theese boost settings are for boost v1.33.1.
//  Boost v1.34.1 claims better WIN_CE support, 
//  it need to be tested if we still need theese options


// Use InterlocedIncrement and critical sections declarations from windows.h
// rather than boost's own declarations
#define BOOST_USE_WINDOWS_H

// Force regex library will use C locale
#define BOOST_REGEX_USE_C_LOCALE

// Disable STL locale usage for windows mobile
#define BOOST_NO_STD_LOCALE

#endif //WINCE


#include <boost/assert.hpp>
#include <boost/smart_ptr.hpp>
//#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/regex.hpp>

//ignoring some warnings in boost/format
#pragma warning(push)

//C4996: deprecated function
// boost 1.33.1 uses deprecated std::char_traits<char>::copy
#pragma warning(disable : 4996)

//C4800: 'type' : forcing value to bool 'true' or 'false' (performance warning)
#pragma warning(disable : 4800)

#include <boost/format.hpp>

#pragma warning(pop)


