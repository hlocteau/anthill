#include <io/GatherFolderImg.hpp>
#include <io/Pgm3dFactory.h>
#include <utils.h>

//namespace io {
GatherFolderImg::GatherFolderImg( const fs::path &folderpath ) {
	_folderpath = folderpath ;
	_minFrequencyMask = 100 ;
	computeDimensions() ;	
}

GatherFolderImg::~GatherFolderImg() {
}
void GatherFolderImg::computeDimensions() {
	int n_slices = -1 ;
	fs::path filePath;
	do {
		n_slices++ ;
		filePath = _folderpath ;
		filePath /= QString("slice-%1.pgm").arg( n_slices, 0, 10 ).toStdString() ;
	} while ( boost::filesystem::exists( filePath ) ) ;
	n_slices-- ;
	filePath = _folderpath ;
	filePath /= QString("slice-%1.pgm").arg( n_slices, 0, 10 ).toStdString() ;
	map<int, QString > desc ;
	QFile imagefile( QString("%1").arg(filePath.string().c_str() ) ) ;
	if ( /*io::*/pgmheader( imagefile, desc ) ) {
		int n_cols = desc[ PGM_HEADER_IMAGE_WIDTH ].toInt() ;
		int n_rows = desc[ PGM_HEADER_IMAGE_HEIGHT ].toInt() ;
		_scene = BillonTpl< char >( n_rows, n_cols, n_slices+1 ) ;
		_scene.fill(0);
	} else {
		std::cerr<<desc[ PGM_HEADER_ERROR ].toStdString()<<std::endl;
	}
	imagefile.close() ;
	std::cerr<<__FUNCTION__<<" : "<<_scene.n_rows<<" x "<<_scene.n_cols<<" x "<<_scene.n_slices<<"["<< n_slices<<"]"<<std::endl;
}
void GatherFolderImg::computeMask( ) {
	_mask = arma::zeros< arma::Mat< char > >( _scene.n_rows, _scene.n_cols ) ;
	
	if ( _minFrequencyMask > 100 ) return ;
	
	arma::Mat< short > counter = arma::zeros< arma::Mat< short > > ( _scene.n_rows, _scene.n_cols ) ;

	int x, y, slice, u ;
	boost::filesystem::path filePath;
	Pgm3dFactory<char> factory ;
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
	for ( slice = 0 ; slice < _scene.n_slices ; slice++ ) {
		filePath = _folderpath ;
		filePath /= QString("slice-%1.pgm").arg( slice, 0, 10 ).toStdString() ;
		BillonTpl<char> *image = factory.read( QString( filePath.string().c_str() ) );
		*image *= -1 ;

		for ( y = 0 ; y < counter.n_rows ; y++ )
			for ( x = 0 ; x < counter.n_cols ; x++ )
				counter( y, x ) += (*image)( y,x,0 ) ;
		delete image ;
	}
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
	short th = (_scene.n_slices*_minFrequencyMask)/100 ;
	for ( y = 0 ; y < _mask.n_rows ; y++ )
		for ( x = 0 ; x < _mask.n_cols ; x++ )
			if ( counter( y, x ) >= th )
				_mask( y, x ) = 1 ;
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
	arma::Mat<char> *dilMask = dilate( _mask, 4, 4 ) ;
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
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
std::cerr<<__FILE__<<" @ line "<<__LINE__<<std::endl;
	if ( min( minDist[0], minDist[2] ) < min( minDist[1], minDist[3] ) ) {
		/// the support is either at the left or the right side
		if ( minDist[0] < minDist[2] ) {
			/// left side
			for ( y = 0 ; y < _mask.n_rows ; y++ )
				for ( x = _mask.n_cols-1 ; x >= 0 ; x-- ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = 0 ; u < x ; u++ )
						_mask(y,u) = 1 ;
					break ;
				}
		} else {
			/// right side
			for ( y = 0 ; y < _mask.n_rows ; y++ )
				for ( x = 0 ; x < _mask.n_cols ; x++ ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = x ; u < _mask.n_cols ; u++ )
						_mask(y,u) = 1 ;
					break ;
				}
		}
	} else {
		/// the support is either at the top or the bottom side
		if ( minDist[1] < minDist[3] ) {
			/// top side
			for ( x = 0 ; x < _mask.n_cols ; x++ )
				for ( y = _mask.n_rows-1 ; y >= 0 ; y-- ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = 0 ; u < y ; u++ )
						_mask(u,x) = 1 ;
					break ;
				}
		} else {
			/// bottom side
			for ( x = 0 ; x < _mask.n_cols ; x++ )
				for ( y = 0 ; y < _mask.n_rows ; y++ ) {
					if ( _mask(y,x) == 0 ) continue ;
					for ( u = x ; u < _mask.n_rows ; u++ )
						_mask(u,x) = 1 ;
					break ;
				}
		}
		
	}
	arma::uvec q=arma::find( _mask ) ;
std::cerr<<__FILE__<<" @ line "<<__LINE__<<" .. "<< q.n_elem<<" (mask) min="<<(int)arma::min(arma::min(_mask))<<" max="<<(int)arma::max(arma::max(_mask))<<std::endl;
	_mask = 1 - _mask ;
	q=arma::find( _mask ) ;
std::cerr<<__FILE__<<" @ line "<<__LINE__<<" .. "<<q.n_elem<<" (mask) min="<<(int)arma::min(arma::min(_mask))<<" max="<<(int)arma::max(arma::max(_mask))<<std::endl;
}

bool GatherFolderImg::load( int minFrequencyMask ) {
	if ( _scene.is_empty() ) return false ;
	_minFrequencyMask = minFrequencyMask ;
	computeMask() ;
	
	Pgm3dFactory<char> factory ;
	if ( false ) {
		BillonTpl<char> dummyMask3d( _mask.n_rows, _mask.n_cols, 1 ) ;
		dummyMask3d.slice(0) = _mask ;
		dummyMask3d.setMinValue(0);
		dummyMask3d.setMaxValue(1);
		IOPgm3d< char, qint8, false>::write( dummyMask3d, QString("/tmp/mask.pgm") ) ;
	}
	fs::path filePath ;
	char 	gMax = 0,
			gMin = 1 ;
	for ( int slice = 0 ; slice < _scene.n_slices ;slice++ ) {
		filePath = _folderpath ;
		filePath /= QString("slice-%1.pgm").arg( slice, 0, 10 ).toStdString() ;
		BillonTpl<char> *im = factory.read( QString(filePath.string().c_str()) ) ;
		im->slice(0) *= -1 ;
		_scene.slice( slice ) = (*im).slice(0) % _mask ;
		
		if ( slice ==364 && false ) {
			BillonTpl<char> dummyMask3d( _mask.n_rows, _mask.n_cols, 1 ) ;
			dummyMask3d.slice(0) = _scene.slice( slice ) ;
			dummyMask3d.setMinValue(0);
			dummyMask3d.setMaxValue(1);
			IOPgm3d< char, qint8, false>::write( dummyMask3d, QString("/tmp/slice364withmask.pgm") ) ;
		}
		gMin = min( gMin, arma::min(arma::min(_scene.slice( slice ))) ) ;
		gMax = max( gMax, arma::max(arma::max(_scene.slice( slice ))) ) ;
		//arma::uvec q=arma::find( _scene.slice( slice ) ) ;
		//std::cout<< q.n_elem<<std::endl;
		delete im ;
	}
std::cerr<<__FILE__<<" @ line "<<__LINE__<<" scene min="<<(int)gMin<<" max="<<(int)gMax<<std::endl;
	return true ;
}
//}
