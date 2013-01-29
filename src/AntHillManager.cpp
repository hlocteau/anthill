#include <AntHillManager.hpp>
#include <io/IOPgm3d.h>
#include <QDir>
#include "dicomreader.h"
#include <io/Pgm3dFactory.h>

#define EXPORT_ROOT_FOLDER_NAME 			"outputData"
#define EXPORT_FOLDER_NAME 					"serie_%1"
#define INDEX_SERIE_POSITION				6
#define EXPORT_FILE_NAME					"input.pgm3d"
#define PROJECT_FILE_NAME					"serie_%1.xml"
#define BINARIZATION_OUTPUT_FILE_NAME		"binary.pgm3d"

#define SIGNED_TINY_INT		1
#define UNSIGNED_TINY_INT	2
#define SIGNED_SHORT_INT	3
#define UNSIGNED_SHORT_INT	4
#define SIGNED_INT			5
#define UNSIGNED_INT		6

#define MEANING_BILEVEL		1
#define MEANING_FEATURE		2
#define MEANING_LABEL		3

#define ANTHILL_INDEX_FILENAME  0
#define ANTHILL_INDEX_MEANING   1
#define ANTHILL_INDEX_ENCODING  2
#define ANTHILL_INDEX_DIMENSION 3
#define ANTHILL_INDEX_OFFSET    4

//template <> QString IOPgm3d<arma::s32,qint32,true>::_header = QString( "PB" ) ;
//template <> QString IOPgm3d<arma::s32,qint32,false>::_header = QString( "P8" ) ;
template <> QString IOPgm3d<arma::u32,qint32,true>::_header = QString( "PB" ) ;
template <> QString IOPgm3d<arma::u32,qint32,false>::_header = QString( "P8" ) ;

//template <> QString IOPgm3d<arma::s8,char,true>::_header = QString( "P2" ) ;
//template <> QString IOPgm3d<arma::s8,qint8,false>::_header = QString( "P5" ) ;
template <> QString IOPgm3d<arma::u8,char,true>::_header = QString( "P2" ) ;
template <> QString IOPgm3d<arma::u8,qint8,false>::_header = QString( "P5" ) ;

template <> QString IOPgm3d<arma::s16,qint32,false>::_header = QString( "P8" ) ;
template <> QString IOPgm3d<arma::s16,qint32,true>::_header = QString( "PB" ) ;
template <> QString IOPgm3d<arma::u16,qint32,false>::_header = QString( "P8" ) ;
template <> QString IOPgm3d<arma::u16,qint32,true>::_header = QString( "PB" ) ;

bool AntHillManager::isContentOnly( QMap< QString, QString >::ConstIterator &res ) const {
	assert( res.key().startsWith("result") ) ;
	assert( res.value().count(";") == 4 ) ;
	return ( res.value().split(";").at(ANTHILL_INDEX_MEANING) == ANTHILL_TAG_FEATURE ) ;
}

bool AntHillManager::isColorSelectionAllowed( QMap< QString, QString >::ConstIterator &res ) const {
	assert( res.key().startsWith("result") ) ;
	assert( res.value().count(";") == 4 ) ;
	return !( res.value().split(";").at(ANTHILL_INDEX_MEANING) == ANTHILL_TAG_FEATURE ) ;
}

void AntHillManager::getSize( QMap< QString, QString >::ConstIterator &res, uint &n_rows, uint &n_cols, uint &n_slices ) const {
	assert( res.key().startsWith("result") ) ;
	assert( res.value().count(";") == 4 ) ;
	
	QStringList dims = res.value().split(";").at(ANTHILL_INDEX_DIMENSION).split(" ") ;
	n_rows = dims.at(0).toInt() ;
	n_cols = dims.at(1).toInt() ;
	n_slices = dims.at(2).toInt() ;
}

void AntHillManager::getOffset( QMap< QString, QString >::ConstIterator &res, uint &row, uint &col, uint &slice ) const {
	assert( res.key().startsWith("result") ) ;
	assert( res.value().count(";") == 4 ) ;
	
	QStringList dims = res.value().split(";").at(ANTHILL_INDEX_OFFSET).split(" ") ;
	row = dims.at(0).toInt() ;
	col = dims.at(1).toInt() ;
	slice = dims.at(2).toInt() ;
}

bool AntHillManager::draw( const QString &resname, arma::Mat<uint8_t> &image, uint8_t axis, uint16_t coordinate, const Interval<int32_t> &range,bool normalize ) {
	QStringList info ;
	if ( !_prop.contains( resname ) ) {
		/// retrieve from _project the encoding
		QMap< QString, QMap<QString,QString> >::ConstIterator resIter = _project->process_begin(),
		                                                      resEnd = _project->process_end() ;
		QMap<QString,QString>::ConstIterator paramIter, paramEnd ;
		while ( resIter != resEnd ) {
			paramIter = resIter.value().begin() ;
			paramEnd = resIter.value().end() ;
			for ( ; paramIter != paramEnd ; paramIter++ )
				if ( paramIter.key().startsWith("result") )
					if ( uid( resIter, paramIter ) == resname ) {
						info = paramIter.value().split( ";" ) ;
						break ;
					}
			if ( !info.isEmpty() ) break ;
			resIter++ ;
		}
		if ( resIter == resEnd ) {
			std::cerr<<"[ Error ] : none of the project's ressources is named "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
			return false;
		}
		if ( info.size() != 3 ) {
			std::cerr<<"[ Error ] : wrong syntax for the project file wrt the ressource "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
			return false;
		}
		fs::path respath = _projectLocation ;
		respath /= info.at(0).toStdString() ;

		if ( info.at(1) == ANTHILL_TAG_BILEVEL ) _prop[ resname ]._meaning = MEANING_BILEVEL ;
		else if ( info.at(1) == ANTHILL_TAG_LABEL ) _prop[ resname ]._meaning = MEANING_LABEL ;
		else if ( info.at(1) == ANTHILL_TAG_FEATURE ) _prop[ resname ]._meaning = MEANING_FEATURE ;
		else {
			std::cerr<<"[ Error ] : unknown meaning "<< info.at(1).toStdString()<<" for the project file wrt the ressource "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
		}

		uint8_t type = 0 ;
		if ( info.at(2) == "8s" ) {
			Pgm3dFactory< arma::s8 > factory ;
			_im_8s.append( factory.read( respath.c_str() ) ) ;
			_prop[ resname ]._adr = (void*) _im_8s.back() ;
			type = SIGNED_TINY_INT ;
		} else if ( info.at(2) == "8u" ) {
			Pgm3dFactory< arma::u8 > factory ;
			_im_8u.append( factory.read( respath.c_str() ) ) ;
			_prop[ resname ]._adr = (void*) _im_8u.back() ;
			type = UNSIGNED_TINY_INT ;
		} else if ( info.at(2) == "16s" ) {
			Pgm3dFactory< arma::s16 > factory ;
			_im_16s.append( factory.read( respath.c_str() ) ) ;
			_prop[ resname ]._adr = (void*) _im_16s.back() ;
			type = SIGNED_SHORT_INT ;
		} else if ( info.at(2) == "16u" ) {
			Pgm3dFactory< arma::u16 > factory ;
			_im_16u.append( factory.read( respath.c_str() ) ) ;
			_prop[ resname ]._adr = (void*) _im_16u.back() ;
			type = UNSIGNED_SHORT_INT ;
		} else if ( info.at(2) == "32s" ) {
			Pgm3dFactory< arma::s32 > factory ;
			_im_32s.append( factory.read( respath.c_str() ) ) ;
			_prop[ resname ]._adr = (void*) _im_32s.back() ;
			type = SIGNED_INT ;
		} else if ( info.at(2) == "32u" ) {
			Pgm3dFactory< arma::u32 > factory ;
			_im_32u.append( factory.read( respath.c_str() ) ) ;
			_prop[ resname ]._adr = (void*) _im_32u.back() ;
			type = UNSIGNED_INT ;
		} else {
			std::cerr<<"[ Error ] : unknown format "<< info.at(2).toStdString()<<" for the project file wrt the ressource "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
			return false;
		}
		_prop[ resname ]._type = type ;
	}
	
	if ( _prop[ resname ]._type == SIGNED_TINY_INT ) {
		Interval<arma::s8> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::s8>*) _prop[ resname ]._adr, image, axis, coordinate, cast_range, normalize ) ;
	} else if ( _prop[ resname ]._type == UNSIGNED_TINY_INT ) {
		Interval<arma::u8> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::u8>*) _prop[ resname ]._adr, image, axis, coordinate, cast_range, normalize ) ;
	} else if ( _prop[ resname ]._type == SIGNED_SHORT_INT ) {
		Interval<arma::s16> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::s16>*) _prop[ resname ]._adr, image, axis, coordinate, cast_range, normalize ) ;
	} else if ( _prop[ resname ]._type == UNSIGNED_SHORT_INT ) {
		Interval<arma::u16> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::u16>*) _prop[ resname ]._adr, image, axis, coordinate, cast_range, normalize ) ;
	} else if ( _prop[ resname ]._type == SIGNED_INT ) {
		Interval<arma::s32> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::s32>*) _prop[ resname ]._adr, image, axis, coordinate, cast_range, normalize ) ;
	} else if ( _prop[ resname ]._type == UNSIGNED_INT ) {
		Interval<arma::u32> cast_range( range.min(), range.max() ) ;
		draw( (BillonTpl<arma::u32>*) _prop[ resname ]._adr, image, axis, coordinate, cast_range, normalize ) ;
	}
	//std::cerr<<"[ Info ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
	return true ;
}

void AntHillManager::openInitialInput( Billon ** img ) {
    if ( *img != 0 ) {
        delete *img;
        *img = 0;
    }
    
    fs::path filename = _projectLocation ;
    filename /= EXPORT_FILE_NAME ;
    
    std::cout<<"[ Info ] : opening image file "<<filename.c_str()<<std::endl;
    *img = IOPgm3d< int32_t, qint32, false>::read( QString( "%1").arg( filename.c_str() ) );
}

AntHillManager::~AntHillManager() {
	reset() ;
	delete _project ;
}

void AntHillManager::reset( ) {
	#if ( QT_MAJOR_VERSION > 4 || ( QT_MAJOR_VERSION == 4 && QT_MINOR_VERSION == 8 ) )
	QVector<QString>().swap( _series ) ;
	#else
	_series.clear() ;
	#endif
	_filename = "" ;
	_currentSeries = -1 ;
	if ( _project )
		delete _project ;
	_project = 0 ;
/*
	while ( !_im_8s.isEmpty() )  delete _im_8s.takeLast() ;
	while ( !_im_16s.isEmpty() ) delete _im_16s.takeLast() ;
	while ( !_im_32s.isEmpty() ) delete _im_32s.takeLast() ;
	while ( !_im_8u.isEmpty() )  delete _im_8u.takeLast() ;
	while ( !_im_16u.isEmpty() ) delete _im_16u.takeLast() ;
	while ( !_im_32u.isEmpty() ) delete _im_32u.takeLast() ;
*/

}

fs::path AntHillManager::defaultProjectLocation( ) const {
	fs::path out_directory_path = QDir::homePath().toStdString() ;
    out_directory_path /= EXPORT_ROOT_FOLDER_NAME ;
    return out_directory_path ; //QString("%1").arg( out_directory_path.c_str() ) ;
}

void AntHillManager::setFileName( const QString &filename ) {
	reset() ;
	_projectLocation = filename.toStdString() ;
	_filename = _projectLocation.filename() ;
	_projectLocation = _projectLocation.parent_path() ;
	_series.push_back( QString("%1").arg(_projectLocation.filename().c_str() ) ) ;
	_currentSeries = 0 ;
}

void AntHillManager::importDicom( const QString &folderName ) {
	reset() ;
	
	QMap< QString, QMap< QString, QString > > dictionary_series ;
	DicomReader::enumerate_dicom_series( folderName, dictionary_series ) ;

    fs::path out_directory_path = defaultProjectLocation() ;
    out_directory_path /= fs::path( folderName.toStdString() ).filename() ;
    _projectLocation = out_directory_path ;
    uint iSerie = 0 ;
    QMap< QString, QMap< QString,QString> >::iterator keyDataIter ;
    for ( keyDataIter = dictionary_series.begin() ; keyDataIter != dictionary_series.end() ; keyDataIter++,iSerie++ ) {
		Billon *img = DicomReader::read(folderName,keyDataIter.key().toStdString() );
		int n_rows = img->n_rows ;
		int n_cols = img->n_cols ;
		int n_slices = img->n_slices ;
		fs::path filepath = _projectLocation ;
		filepath /= QString(EXPORT_FOLDER_NAME).arg( iSerie ).toStdString() ;
		if ( !fs::exists( filepath ) ) fs::create_directories( filepath ) ;
		filepath /= EXPORT_FILE_NAME ;
		QString qfilename = QString( "%1" ).arg( filepath.c_str() ) ;
		IOPgm3d< __billon_type__, qint16, false >::write( *img, qfilename ) ;
		delete img ;
		filepath = filepath.parent_path() ;
		filepath /= QString(PROJECT_FILE_NAME).arg( iSerie ).toStdString() ;
		AntHillFile prj ;
		prj.setDicomFolder( folderName ) ;
		prj.setUID( keyDataIter.key() ) ;
		prj.setDictionary( keyDataIter.value() );
		QMap< QString, QString > details ;
		prj.addProcess( "import", details ) ;
		details.insert( "size", QString("%1;%2;%3").arg(n_rows).arg(n_cols).arg(n_slices) ) ;
		details.insert( "result", QString("%1;%2;16u").arg(EXPORT_FILE_NAME).arg(ANTHILL_TAG_FEATURE) ) ;
		prj.save( QString::fromStdString(filepath.c_str()) ) ;
		#if ( QT_MAJOR_VERSION > 4 || ( QT_MAJOR_VERSION == 4 && QT_MINOR_VERSION == 8 ) )
		QMap< QString,QString >().swap( keyDataIter.value() ) ;
		#else
		keyDataIter.value().clear() ;
		#endif
		_series.push_back( QString(EXPORT_FOLDER_NAME).arg( iSerie ) ) ;
	}
}

bool AntHillManager::binarization( const Billon *data, const Interval<__billon_type__> &range, int th) {
    if ( range.width() == 0 ) return false ;
    fs::path filepath = _projectLocation ;
    filepath /= BINARIZATION_OUTPUT_FILE_NAME ;
    
    BillonTpl<uint8_t> binImage( data->n_rows, data->n_cols, data->n_slices ) ;
    arma::Cube< __billon_type__ >::const_iterator 	in = data->begin(),
													in_end = data->end() ;
    arma::Cube< uint8_t >::iterator out = binImage.begin() ;
    for (  ; in != in_end ; in++,out++ ) {
		if (  range.containsClosed( *in ) ) {
			if ( (int)floor( ( ( *in - range.min() ) * (double)255. ) / range.size() ) >= th )
				*out = 1 ;
			else
				*out = 0 ;
		} else
			*out = 0 ;
    }
    binImage.setMaxValue( 1 ) ;
    IOPgm3d< uint8_t, qint8, false >::write( binImage, QString("%1").arg( filepath.c_str() ) ) ;
    QMap< QString, QString > details ;
    details.insert( "minimum", QString("%1").arg( range.min() ) ) ;
    details.insert( "maximum", QString("%1").arg( range.max() ) ) ;
    details.insert( "threshold", QString("%1").arg( th ) ) ;
    details.insert( "size", QString("%1;%2;%3").arg(data->n_rows).arg(data->n_cols).arg(data->n_slices) ) ;
    details.insert( "result", QString("%1;%2;8u").arg(BINARIZATION_OUTPUT_FILE_NAME).arg(ANTHILL_TAG_BILEVEL) ) ;
    _project->addProcess( "binarisation", details ) ;
    fs::path filename = _projectLocation ;
    filename /= _filename ;
    _project->save( QString("%1").arg( filename.c_str() ) ) ;
    return true ;
}

bool AntHillManager::load( const QString &name ) {
    if ( _filename.empty() ) {
		uint iItemSerie = name.mid( INDEX_SERIE_POSITION ).toInt() ;
		if ( iItemSerie == _currentSeries ) return true ; // no change
	
		std::cout<<"[ Info ] will try to open serie "<<iItemSerie<<std::endl;
	
		QVector< QString >::iterator entry = _series.begin() ;
		uint iEntrySerie = (*entry).mid( INDEX_SERIE_POSITION ).toInt() ;
		while ( iEntrySerie != iItemSerie ) {
			entry++ ;
			if ( entry == _series.end() ) {
				return false ;
			}
			iEntrySerie = (*entry).mid( INDEX_SERIE_POSITION ).toInt() ;
		}
		_currentSeries = iItemSerie ;   
		_projectLocation /= name.toStdString() ;
		_filename = QString( PROJECT_FILE_NAME ).arg( iItemSerie ).toStdString() ; 
	}
	fs::path filepath = _projectLocation ;
	filepath /= _filename ;
	if ( _project )
		delete _project ;
	_project = new AntHillFile ;

	if ( !_project->load( QString( "%1").arg( filepath.c_str() ) ) ) {
		_currentSeries = -1 ;
		delete _project ;
		return false ;
	}
	return true ;
}
