#ifndef QBTD_EXCEPTION_JSONERROR_HPP
#define QBTD_EXCEPTION_JSONERROR_HPP

#include "qbtd/exception.hpp"

namespace qbtd {
namespace exception {

class QBTD_DLL JsonError: public Exception {
public:
	JsonError( int errnum, const char * file, std::size_t line );
	JsonError( const char * message, const char * file, std::size_t line );
	JsonError( const char * message, std::size_t n, const char * file, std::size_t line );
	JsonError( const wchar_t * message, const char * file, std::size_t line );
	JsonError( const wchar_t * message, std::size_t n, const char * file, std::size_t line );
	JsonError( const std::string & message, const char * file, std::size_t line );
	JsonError( const std::wstring & message, const char * file, std::size_t line );
	JsonError( const QString & message, const char * file, std::size_t line );
	virtual ~JsonError() throw();
};

}
}

#endif
