#include <io/GatherFolderImg.hpp>
#include <io/Pgm3dFactory.h>
#include <utils.h>
#include <io/AntHillFile.hpp>
namespace fs = boost::filesystem ;

//namespace io {
GatherFolderImg::GatherFolderImg( const fs::path &folderpath ) {
	_minFrequencyMask = 100 ;

	_folderpath = folderpath ;
	if ( ! fs::is_directory( _folderpath) ) {
		Pgm3dFactory<arma::u8> factory ;
		_scene = factory.read( QString( _folderpath.c_str() ) );
		// _scene 's values have to be either 0 or 1
		if ( _scene->max() > 1 )
			*_scene /= _scene->max() ;
	} else {
		computeDimensions() ;
	}
}

GatherFolderImg::~GatherFolderImg() {
	delete _scene ;
}

void GatherFolderImg::computeDimensions() {
	int n_slices = -1 ;
	fs::path filePath;
	do {
		n_slices++ ;
		filePath = _folderpath ;
		filePath /= QString( ANTHILL_SLICE_NAME ).arg( n_slices, 0, 10 ).toStdString() ;
	} while ( boost::filesystem::exists( filePath ) ) ;
	n_slices-- ;
	filePath = _folderpath ;
	filePath /= QString( ANTHILL_SLICE_NAME ).arg( n_slices, 0, 10 ).toStdString() ;
	map<int, QString > desc ;
	QFile imagefile( filePath.string().c_str() ) ;
	if ( /*io::*/pgmheader( imagefile, desc ) ) {
		int n_cols = desc[ PGM_HEADER_IMAGE_WIDTH ].toInt() ;
		int n_rows = desc[ PGM_HEADER_IMAGE_HEIGHT ].toInt() ;
		_scene = new BillonTpl< arma::u8 >( n_rows, n_cols, n_slices+1 ) ;
		_scene->fill(0);
	} else {
		std::cerr<<desc[ PGM_HEADER_ERROR ].toStdString()<<std::endl;
	}
	imagefile.close() ;
}

void GatherFolderImg::computeMask( bool save_counter ) {
	_mask = arma::zeros< arma::Mat< arma::u8 > >( _scene->n_rows, _scene->n_cols ) ;
	
	if ( _minFrequencyMask > 100 ) {
		_mask = 1 - _mask ;
		return ;
	}
	
	arma::Mat< arma::u16 > counter = arma::zeros< arma::Mat< arma::u16 > > ( _scene->n_rows, _scene->n_cols ) ;

	int x, y, slice, u ;
	boost::filesystem::path filePath;
	Pgm3dFactory<arma::u8> factory ;

	if ( fs::is_directory( _folderpath) ) {
		for ( slice = 0 ; slice < _scene->n_slices ; slice++ ) {
			filePath = _folderpath ;
			filePath /= QString( ANTHILL_SLICE_NAME ).arg( slice, 0, 10 ).toStdString() ;
			BillonTpl<arma::u8> *image = factory.read( QString( filePath.string().c_str() ) );
			*image *= -1 ;

			for ( y = 0 ; y < counter.n_rows ; y++ )
				for ( x = 0 ; x < counter.n_cols ; x++ )
					counter( y, x ) += (*image)( y,x,0 ) ;
			delete image ;
		}
	} else {
		for ( slice = 0 ; slice < _scene->n_slices ; slice++ ) {
			arma::Mat< arma::u8 > image = _scene->slice( slice ) ;
			arma::Mat< arma::u8 >::iterator readIter,
											readEnd = image.end() ;
			arma::Mat< arma::u16 >::iterator writeIter = counter.begin();
			for ( readIter = image.begin() ; readIter != readEnd ; readIter++,writeIter++ )
				*writeIter += *readIter ;
		}
	}
	
	if ( save_counter ) {
		arma::Cube<arma::u16> counter3D( counter.n_rows, counter.n_cols, 1 ) ;
		counter3D.slice(0) = counter ;
		counter3D.slice(0) *= 255 ;
		counter3D.slice(0) /= _scene->n_slices ;
		fs::path premaskfs = _folderpath ;
		premaskfs /= ANTHILL_PRE_MASK_NAME ;
		IOPgm3d<arma::u16,qint8,false>::write( counter3D, QString("%1").arg( premaskfs.string().c_str() ) );
	}
	arma::u16 th = (_scene->n_slices*_minFrequencyMask)/100 ;
	for ( y = 0 ; y < _mask.n_rows ; y++ )
		for ( x = 0 ; x < _mask.n_cols ; x++ )
			if ( counter( y, x ) >= th )
				_mask( y, x ) = 1 ;
	arma::Mat<arma::u8> *dilMask = dilate( _mask, 4, 4 ) ;
	_mask = *dilMask ;
	delete dilMask ;
	
	int minDist[] = { 0,0,0,0 } ;

	for ( y = 0 ; y < _mask.n_rows ; y++ ) 
		for ( x = 0 ; x < _mask.n_cols ; x++ )	{
			if ( _mask(y,x) == 0 ) continue ;
			minDist[0] = max( minDist[0], x ) ;
			minDist[1] = max( minDist[1], y ) ;
			
			minDist[2] = max( minDist[2], (int)_mask.n_cols-x ) ;
			minDist[3] = max( minDist[3], (int)_mask.n_rows-y ) ;
		}
	if ( min( minDist[0], minDist[2] ) < min( minDist[1], minDist[3] ) ) {
		// the stand is either at the left or the right side
		if ( minDist[0] < minDist[2] ) {
			// left side
			for ( y = 0 ; y < _mask.n_rows ; y++ )
				for ( x = _mask.n_cols-1 ; x >= 0 ; x-- ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = 0 ; u < x ; u++ )
						_mask(y,u) = 1 ;
					break ;
				}
		} else {
			// right side
			for ( y = 0 ; y < _mask.n_rows ; y++ )
				for ( x = 0 ; x < _mask.n_cols ; x++ ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = x ; u < _mask.n_cols ; u++ )
						_mask(y,u) = 1 ;
					break ;
				}
		}
	} else {
		// the stand is either at the top or the bottom side
		if ( minDist[1] < minDist[3] ) {
			// top side
			for ( x = 0 ; x < _mask.n_cols ; x++ )
				for ( y = _mask.n_rows-1 ; y >= 0 ; y-- ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = 0 ; u < y ; u++ )
						_mask(u,x) = 1 ;
					break ;
				}
		} else {
			// bottom side
			for ( x = 0 ; x < _mask.n_cols ; x++ )
				for ( y = 0 ; y < _mask.n_rows ; y++ ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = y ; u < _mask.n_rows ; u++ )
						_mask(u,x) = 1 ;
					break ;
				}
		}
		
	}
	_mask = 1 - _mask ;
}

bool GatherFolderImg::load( int minFrequencyMask, bool save_counter ) {
	if ( _scene->is_empty() ) return false ;
	_minFrequencyMask = minFrequencyMask ;
	computeMask( save_counter ) ;
	
	Pgm3dFactory<arma::u8> factory ;

	fs::path filePath ;
	for ( int slice = 0 ; slice < _scene->n_slices ;slice++ ) {
		if ( fs::is_directory( _folderpath) ) {
			filePath = _folderpath ;
			filePath /= QString( ANTHILL_SLICE_NAME ).arg( slice, 0, 10 ).toStdString() ;
			BillonTpl<arma::u8> *im = factory.read( QString(filePath.string().c_str()) ) ;
			im->slice(0) *= -1 ;
			_scene->slice( slice ) = (*im).slice(0) % _mask ;
			delete im ;
		} else {
			_scene->slice( slice ) = _scene->slice( slice ) % _mask ;
		}
	}

	return true ;
}

//}
