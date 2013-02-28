#include <AntHillManager.hpp>
#include <io/IOPgm3d.h>
#include <QDir>
#include <io/dicomreader.h>
#include <io/Pgm3dFactory.h>

#define EXPORT_ROOT_FOLDER_NAME 			"outputData"
#define EXPORT_FOLDER_NAME 					"serie_%1"
#define INDEX_SERIE_POSITION				6
#define EXPORT_FILE_NAME					"input.pgm3d"
#define PROJECT_FILE_NAME					"serie_%1.xml"
#define BINARIZATION_OUTPUT_FILE_NAME		"binary.pgm3d"



#define MEANING_BILEVEL		1
#define MEANING_FEATURE		2
#define MEANING_LABEL		3

#define ANTHILL_INDEX_FILENAME  0
#define ANTHILL_INDEX_MEANING   1
#define ANTHILL_INDEX_ENCODING  2
#define ANTHILL_INDEX_DIMENSION 3
#define ANTHILL_INDEX_OFFSET    4



bool AntHillManager::isContentOnly( QMap< QString, QString >::ConstIterator &res ) const {
	assert( res.key().startsWith(ANTHILL_DEFAULT_OUTPUT_NAME) ) ;
	assert( res.value().count(";") > ANTHILL_INDEX_MEANING ) ;
	return ( res.value().split(";").at(ANTHILL_INDEX_MEANING) == ANTHILL_TAG_FEATURE ) ;
}

bool AntHillManager::isColor( QMap< QString, QString >::ConstIterator &res ) const {
	assert( res.key().startsWith(ANTHILL_DEFAULT_OUTPUT_NAME) ) ;
	assert( res.value().count(";") > ANTHILL_INDEX_MEANING ) ;
	return ( res.value().split(";").at(ANTHILL_INDEX_MEANING) != ANTHILL_TAG_FEATURE ) ;
}

bool AntHillManager::isColorSelectionAllowed( QMap< QString, QString >::ConstIterator &res ) const {
	assert( res.key().startsWith(ANTHILL_DEFAULT_OUTPUT_NAME) ) ;
	assert( res.value().count(";") > ANTHILL_INDEX_MEANING ) ;
	return ( res.value().split(";").at(ANTHILL_INDEX_MEANING) == ANTHILL_TAG_BILEVEL ) ;
}

void AntHillManager::getSize( QMap< QString, QString >::ConstIterator &res, uint &n_rows, uint &n_cols, uint &n_slices ) const {
	assert( res.key().startsWith(ANTHILL_DEFAULT_OUTPUT_NAME) ) ;
	assert( res.value().count(";") == 4 ) ;
	
	QStringList dims = res.value().split(";").at(ANTHILL_INDEX_DIMENSION).split(" ") ;
	n_rows = dims.at(0).toInt() ;
	n_cols = dims.at(1).toInt() ;
	n_slices = dims.at(2).toInt() ;
}

void AntHillManager::getOffset( QMap< QString, QString >::ConstIterator &res, uint &row, uint &col, uint &slice ) const {
	assert( res.key().startsWith(ANTHILL_DEFAULT_OUTPUT_NAME) ) ;
	assert( res.value().count(";") == 4 ) ;
	
	QStringList dims = res.value().split(";").at(ANTHILL_INDEX_OFFSET).split(" ") ;
	row = dims.at(0).toInt() ;
	col = dims.at(1).toInt() ;
	slice = dims.at(2).toInt() ;
}

bool AntHillManager::getSize( uint &n_rows, uint &n_cols, uint &n_slices ) {
	QString main_input_name = QString("%1:%2").arg( ANTHILL_OVERALL_INPUT_PROCESS_NAME ).arg( ANTHILL_DEFAULT_OUTPUT_NAME ) ;
	assert( load_ressource( main_input_name ) ) ;
	
	switch ( _prop[ main_input_name ]._type ) {
	case SIGNED_TINY_INT :
		{
			BillonTpl< arma::s8 > *img = (BillonTpl< arma::s8 >*) _prop[ main_input_name ]._adr ;
			n_rows = img->n_rows ;
			n_cols = img->n_cols ;
			n_slices = img->n_slices ;
		}
		break;
	case UNSIGNED_TINY_INT :
		{
			BillonTpl< arma::u8 >  *img = (BillonTpl< arma::u8>*) _prop[ main_input_name ]._adr ;
			n_rows = img->n_rows ;
			n_cols = img->n_cols ;
			n_slices = img->n_slices ;
		}
		break;
	case SIGNED_SHORT_INT :
		{
			BillonTpl< arma::s16 > *img = (BillonTpl< arma::s16>*) _prop[ main_input_name ]._adr ;
			n_rows = img->n_rows ;
			n_cols = img->n_cols ;
			n_slices = img->n_slices ;
		}
		break;
	case UNSIGNED_SHORT_INT :
		{
			BillonTpl< arma::u16 >  *img = (BillonTpl< arma::u16>*) _prop[ main_input_name ]._adr ;
			n_rows = img->n_rows ;
			n_cols = img->n_cols ;
			n_slices = img->n_slices ;
		}
		break;
	case SIGNED_INT :
		{
			BillonTpl< arma::s32 >  *img = (BillonTpl< arma::s32>*) _prop[ main_input_name ]._adr ;
			n_rows = img->n_rows ;
			n_cols = img->n_cols ;
			n_slices = img->n_slices ;
		}
		break;
	case UNSIGNED_INT :
		{
			BillonTpl< arma::u32 >  *img = (BillonTpl< arma::u32>*) _prop[ main_input_name ]._adr ;
			n_rows = img->n_rows ;
			n_cols = img->n_cols ;
			n_slices = img->n_slices ;
		}
		break;
	default:
		std::cerr<<"[ Error ] : unmanaged type "<< _prop[ main_input_name ]._type <<" for the project file wrt the ressource "<<main_input_name.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
		return false;
	}
	return true ;
}

bool AntHillManager::load_ressource( const QString &resname ) {
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
				if ( paramIter.key().startsWith(ANTHILL_DEFAULT_OUTPUT_NAME) )
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
		if ( info.size() < 3 ) {
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
			if ( _prop[ resname ]._meaning == MEANING_BILEVEL ) {
				if ( _im_8s.back()->max() != 1 )
					*_im_8s.back() /= _im_8s.back()->max() ;
			}
			_prop[ resname ]._adr = (void*) _im_8s.back() ;
			type = SIGNED_TINY_INT ;
		} else if ( info.at(2) == "8u" ) {
			Pgm3dFactory< arma::u8 > factory ;
			_im_8u.append( factory.read( respath.c_str() ) ) ;
			if ( _prop[ resname ]._meaning == MEANING_BILEVEL ) {
				if ( _im_8u.back()->max() != 1 )
					*_im_8u.back() /= _im_8u.back()->max() ;
			}
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
		
		_prop[ resname ]._size = new uint[ 3 ] ;
		_prop[ resname ]._size[ 0 ] = _prop[ resname ]._size[ 1 ] = _prop[ resname ]._size[ 2 ] = 0 ;
		if ( info.size() > ANTHILL_INDEX_DIMENSION ) {
			QStringList dims = info.at(ANTHILL_INDEX_DIMENSION).split(" ") ;
			if ( dims.size() != 3 ) {
				std::cerr<<"[ Warning ] : can not read dimension field (buffer : "<<info.at(ANTHILL_INDEX_DIMENSION).toStdString()<<") for ressource "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
			} else {
				_prop[ resname ]._size[ 0 ] = dims.at(0).toInt();
				_prop[ resname ]._size[ 1 ] = dims.at(1).toInt();
				_prop[ resname ]._size[ 2 ] = dims.at(2).toInt();
			}
		}

		_prop[ resname ]._offset = new uint[ 3 ] ;
		_prop[ resname ]._offset[ 0 ] = _prop[ resname ]._offset[ 1 ] = _prop[ resname ]._offset[ 2 ] = 0 ;
		if ( info.size() > ANTHILL_INDEX_OFFSET ) {
			QStringList dims = info.at(ANTHILL_INDEX_OFFSET).split(" ") ;
			if ( dims.size() != 3 ) {
				std::cerr<<"[ Warning ] : can not read offset field (buffer : "<<info.at(ANTHILL_INDEX_OFFSET).toStdString()<<") for ressource "<<resname.toStdString()<<" Function "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
			} else {
				_prop[ resname ]._offset[ 0 ] = dims.at(0).toInt();
				_prop[ resname ]._offset[ 1 ] = dims.at(1).toInt();
				_prop[ resname ]._offset[ 2 ] = dims.at(2).toInt();
			}
		}
	}
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
	
	while ( !_im_8s.isEmpty() )  delete _im_8s.takeLast() ;
	while ( !_im_16s.isEmpty() ) delete _im_16s.takeLast() ;
	while ( !_im_32s.isEmpty() ) delete _im_32s.takeLast() ;
	while ( !_im_8u.isEmpty() )  delete _im_8u.takeLast() ;
	while ( !_im_16u.isEmpty() ) delete _im_16u.takeLast() ;
	while ( !_im_32u.isEmpty() ) delete _im_32u.takeLast() ;

	while ( !_prop.isEmpty() ) {
		delete [] _prop.begin().value()._offset ;
		delete [] _prop.begin().value()._size ;
		_prop.erase( _prop.begin() ) ;
	}
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
	DicomReader Dicom( folderName ) ;
	
    fs::path out_directory_path = defaultProjectLocation() ;
    out_directory_path /= fs::path( folderName.toStdString() ).filename() ;
    _projectLocation = out_directory_path ;
    uint iSerie = 0 ;
    QMap< QString, QMap< QString,QString> >::ConstIterator keyDataIter ;
    for ( keyDataIter = Dicom.begin() ; keyDataIter != Dicom.end() ; keyDataIter++,iSerie++ ) {
		BillonTpl<arma::s16> *img = Dicom.get<arma::s16>( keyDataIter );
		int n_rows = img->n_rows ;
		int n_cols = img->n_cols ;
		int n_slices = img->n_slices ;
		fs::path filepath = _projectLocation ;
		filepath /= QString(EXPORT_FOLDER_NAME).arg( iSerie ).toStdString() ;
		if ( !fs::exists( filepath ) ) fs::create_directories( filepath ) ;
		filepath /= EXPORT_FILE_NAME ;
		QString qfilename = QString( "%1" ).arg( filepath.c_str() ) ;
		IOPgm3d< arma::s16, qint16, false >::write( *img, qfilename ) ;
		delete img ;
		filepath = filepath.parent_path() ;
		filepath /= QString(PROJECT_FILE_NAME).arg( iSerie ).toStdString() ;
		AntHillFile prj ;
		prj.setDicomFolder( folderName ) ;
		prj.setUID( keyDataIter.key() ) ;
		prj.setDictionary( keyDataIter.value() );
		QMap< QString, QString > details ;
		details.insert( ANTHILL_DEFAULT_OUTPUT_NAME, QString("%1;%2;16s;%3 %4 %5").arg(EXPORT_FILE_NAME).arg(ANTHILL_TAG_FEATURE).arg(n_rows).arg(n_cols).arg(n_slices) ) ;
		prj.addProcess( ANTHILL_OVERALL_INPUT_PROCESS_NAME, details ) ;
		prj.save( QString::fromStdString(filepath.c_str()) ) ;
		_series.push_back( QString(EXPORT_FOLDER_NAME).arg( iSerie ) ) ;
	}
}

bool AntHillManager::binarization( const Interval<arma::s16> &range, int th) {
    if ( range.width() == 0 ) return false ;
    fs::path filepath = _projectLocation ;
    filepath /= BINARIZATION_OUTPUT_FILE_NAME ;
    
    uint n_rows, n_cols, n_slices ;
    getSize(n_rows, n_cols, n_slices ) ;
    
    BillonTpl<arma::u8> binImage( n_rows, n_cols, n_slices ) ;
    bool valid ;
    BillonTpl<arma::s16> *data = getRessourceByUID<arma::s16>( QString("%1:%2").arg( ANTHILL_OVERALL_INPUT_PROCESS_NAME ).arg( ANTHILL_DEFAULT_OUTPUT_NAME ), valid ) ;
    assert( valid ) ;
    arma::Cube< arma::s16 >::const_iterator 	in = data->begin(),
												in_end = data->end() ;
    arma::Cube< arma::u8 >::iterator out = binImage.begin() ;
    for (  ; in != in_end ; in++,out++ ) {
		if (  range.containsClosed( *in ) ) {
			if ( (int)floor( ( ( *in - range.min() ) * (double)255. ) / range.size() ) >= th )
				*out = 1 ;
			else
				*out = 0 ;
		} else {
			*out = range.max() < *in ? 1 : 0 ;
		}
    }
    binImage.setMaxValue( 1 ) ;
    IOPgm3d< arma::u8, qint8, false >::write( binImage, QString("%1").arg( filepath.c_str() ) ) ;
    QMap< QString, QString > details ;
    details.insert( "minimum", QString("%1").arg( range.min() ) ) ;
    details.insert( "maximum", QString("%1").arg( range.max() ) ) ;
    details.insert( "threshold", QString("%1").arg( th ) ) ;
    details.insert( ANTHILL_DEFAULT_OUTPUT_NAME, QString("%1;%2;8u;%3 %4 %5").arg(BINARIZATION_OUTPUT_FILE_NAME).arg(ANTHILL_TAG_BILEVEL).arg(data->n_rows).arg(data->n_cols).arg(data->n_slices) ) ;
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
