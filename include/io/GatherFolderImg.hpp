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
	/**
	 * \param [in] a filepath to either a folder containing individual slices saved as 2D bi-level pgm or a 3D bi-level pgm
	 * \see AntHillFile.hpp for naming conventions
	 */
	GatherFolderImg( const fs::path &folderpath ) ;
	~GatherFolderImg() ;
	
	/**
	 * load the 3D image from either a serie of 2D pgm or a single 3D pgm and apply a mask on it if the threshold is smaller than 100
	 * \param [in] minFrequencyMask the threshold (percentage * 100 ) of foreground voxels on a given (x,y)
	 * \param [in] save_counter flag passed to \a compute_mask
	 * \see computeMask
	 * \return true on success
	 */
	bool load( int minFrequencyMask=100, bool save_counter=false ) ;
	
	const arma::Mat< arma::u8 > & mask( ) const {
		return _mask ;
	}
	const BillonTpl< arma::u8 > & scene( ) const {
		return *_scene ;
	}
protected:
	/**
	 * parse the input folder \a _folderpath to define the size of the resulting 3D image \a _scene
	 */
	void computeDimensions() ;
	/**
	 * \brief Computes the mask to be applied on the given 3D bi-level image.
	 * 
	 * \param [in] save_counter flag conditioning the export of the preimage of the mask
	 * 
	 * Considering all slices, it first computes per each (x,y) the foreground - frequency.
	 * Binarizing this frequency map enables to initialize the mask.
	 * Next, a morphological dilatation is applied and a projection to the closest side 
	 * (top/bottom/left/right) finalizes the definition of the mask.
	 */
	void computeMask( bool save_counter ) ;
private:
	fs::path 			  _folderpath ;
	int					  _minFrequencyMask ;
	BillonTpl< arma::u8 > *_scene ;
	arma::Mat< arma::u8 > _mask ;
} ;
//}
#endif
