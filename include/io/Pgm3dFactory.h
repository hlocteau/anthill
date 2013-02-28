#ifndef PGM_3D_FACTORY_HEADER
#define PGM_3D_FACTORY_HEADER

#include <io/IOPgm3d.h>

//namespace io {
	/**
	 * \brief this interface is the best choice for reading an 3D pgm as the only matter is the type of container's elements, do not paying attention to the file encoding.
	 * \tparam T type used for the voxels
	 */
	template <typename T> class Pgm3dFactory {
		public :
		typedef 		enum { ASCII_CHAR, BINARY_CHAR, BINARY_CHAR_SECOND, BINARY_INT, BINARY_FLOAT, ASCII_FLOAT, ASCII_INT, BINARY_DOUBLE, ASCII_DOUBLE } TEncoding;
		typedef			T	value_type ;
		BillonTpl<T> *read( const QString &fileName ) ;
		/**
		 * \brief correct 32 bits integers' values when the file is issued from a program that do not manage endian as espected.
		 * \note use this method just after opening the image as the correction is achieved wrt the last type been discovered while reading a 3D pgm.
		 * \code
	Pgm3dFactory< arma::u32 > factory ;
	BillonTpl< arma::u32> *im32 = factory.read ( firstfilename ) ;  // reads a true 32 bits image
	BillonTpl< arma::u32> *im8  = factory.read ( secondfilename ) ; // reads a 8 bits image and stores it as a 32 bits image
	factory.correctEncoding( *im32 ) ;                              // wrong as reading im8 makes factory pointing to 8 bits images
			\endcode
		 * \warning it is only effective if the file format type and the container type are the same!
		 * \todo add a parameter to the read() method to correct endianness as soon as possible, till it is meaningfull and when it is required
		 */
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
