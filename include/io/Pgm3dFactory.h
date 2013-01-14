#ifndef PGM_3D_FACTORY_HEADER
#define PGM_3D_FACTORY_HEADER

#include <io/IOPgm3d.h>

//namespace io {
	template <typename T> class Pgm3dFactory {
		public :
		typedef 		enum { ASCII_CHAR, BINARY_CHAR, BINARY_CHAR_SECOND, BINARY_INT, BINARY_FLOAT, ASCII_FLOAT, ASCII_INT, BINARY_DOUBLE, ASCII_DOUBLE } TEncoding;
		typedef			T	value_type ;
		BillonTpl<T> *read( const QString &fileName ) ;
		void correctEncoding( BillonTpl<T> * img ) ;
		bool was_ascii() const { return _lasttype==ASCII_CHAR || _lasttype==ASCII_FLOAT || _lasttype==ASCII_INT || _lasttype==ASCII_DOUBLE ; }
		bool was_binary() const { return !was_ascii() ; }
		bool was_char() const { return _lasttype==ASCII_CHAR||_lasttype==BINARY_CHAR||_lasttype==BINARY_CHAR_SECOND ; }
		bool was_integer() const { return _lasttype==ASCII_INT||_lasttype==BINARY_INT ; }
		private :
		TEncoding _lasttype ;
	} ;
//}

#include <io/Pgm3dFactory.ih>
#endif
