//os_memoryError.h
#ifndef OS_MEMORYERROR
#define OS_MEMORYERROR

#include <stdexcept>

class memory_error:std::logic_error    //从logic_error派生自定义异常类
{
private:
	std::string s;
public:
    memory_error(const std::string &s):std::logic_error(s){this->s=s;}
	std::string toString(){return s;}
    ~memory_error() throw(){}
};

#endif