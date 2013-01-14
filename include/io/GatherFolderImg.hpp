#ifndef GATHER_FOLDER_IMAGES_HEADER
#define GATHER_FOLDER_IMAGES_HEADER

#include <armadillo>
#include <def_billon.h>
#include <billon.h>
#include <boost/filesystem.hpp>
//namespace io {
namespace fs=boost::filesystem;

class GatherFolderImg {
public:
	GatherFolderImg( const fs::path &folderpath ) ;
	~GatherFolderImg() ;
	
	bool load( int minFrequencyMask=100 ) ;
	
	const arma::Mat< char > & mask( ) const {
		return _mask ;
	}
	const BillonTpl< char > & scene( ) const {
		return _scene ;
	}
protected:
	void computeDimensions() ;
	void computeMask() ;
private:
	fs::path 			_folderpath ;
	int					_minFrequencyMask ;
	BillonTpl< char > 	_scene ;
	arma::Mat< char > 	_mask ;
} ;
//}
#endif
