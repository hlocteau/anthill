/**
 * \file writerBinaryPgm3D.h
 */
#ifndef WRITER_BINARY_PGM_3D_HEADER
#define WRITER_BINARY_PGM_3D_HEADER
#include <QString>
#include <DGtal/helpers/StdDefs.h>
#include <def_billon.h>
class IOPgm3d {
public:
	IOPgm3d() ;
	static bool write( const Billon &img, QString fileName ) ;
	static bool write( const std::vector< DGtal::Z3i::Object6_18*> &objCC, QString fileName ) ;
} ;

#endif
