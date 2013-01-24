#include <AntHillManager.hpp>
#include <io/IOPgm3d.h>
#include <QDir>
#include "dicomreader.h"

#define EXPORT_ROOT_FOLDER_NAME "outputData"
#define EXPORT_FOLDER_NAME 		"serie_%1"
#define INDEX_SERIE_POSITION	6
#define EXPORT_FILE_NAME		"input.pgm3d"
#define PROJECT_FILE_NAME		"serie_%1.xml"

void AntHillManager::openInitialInput( Billon ** img ) {
    if ( *img != 0 )	{
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
	QVector<QString>().swap( _series ) ;
	_filename = "" ;
	_currentSeries = -1 ;
	if ( _project )
		delete _project ;
	_project = 0 ;
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
		fs::path filepath = _projectLocation ;
		filepath /= QString(EXPORT_FOLDER_NAME).arg( iSerie ).toStdString() ;
		if ( !fs::exists( filepath ) ) fs::create_directories( filepath ) ;
		filepath /= EXPORT_FILE_NAME ;
		QString qfilename = QString( "%1" ).arg( filepath.c_str() ) ;
		IOPgm3d< int32_t, qint32, false >::write( *img, qfilename ) ;
		delete img ;
		filepath = filepath.parent_path() ;
		filepath /= QString(PROJECT_FILE_NAME).arg( iSerie ).toStdString() ;
		AntHillFile prj ;
		prj.setDicomFolder( folderName ) ;
		prj.setUID( keyDataIter.key() ) ;
		prj.setDictionary( keyDataIter.value() );
		prj.save( QString::fromStdString(filepath.c_str()) ) ;
		QMap< QString,QString >().swap( keyDataIter.value() ) ;
		_series.push_back( QString(EXPORT_FOLDER_NAME).arg( iSerie ) ) ;
	}
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
