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
	
	const arma::Mat< arma::u8 > & mask( ) const {
		return _mask ;
	}
	const BillonTpl< arma::u8 > & scene( ) const {
		return *_scene ;
	}
protected:
	void computeDimensions() ;
	void computeMask() ;
private:
	fs::path 			_folderpath ;
	int					_minFrequencyMask ;
	BillonTpl< arma::u8 > 	*_scene ;
	arma::Mat< arma::u8 > 	_mask ;
} ;
//}
#endif
