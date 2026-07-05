#ifndef MACROS_HH
#define MACROS_HH

#include <iostream>
#include <cstdlib>

#ifndef ASSERT_ENABLED
#define ASSERT_ENABLED false
#endif

#if (ASSERT_ENABLED)
#define ASSERT(condition) if(!(condition)) {std::cerr << "Error in file " << __FILE__ << " in function " << __FUNCTION__ << " at line " << __LINE__ << "!" << std::endl; abort(); } ((void)0)
#else
#define ASSERT(condition) if(!(false)) {((void)0); } ((void)0)
#endif


#endif