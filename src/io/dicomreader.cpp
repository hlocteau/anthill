#include "dicomreader.h"

DicomReader::DicomReader( const QString &repository ) {
	_repository = repository ;
	enumerate_dicom_series( ) ;
}

void DicomReader::enumerate_dicom_series( ) {
	
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetUseSeriesDetails( true ); // to handle distinct series within a single repository
	nameGenerator->SetDirectory(_repository.toStdString());

	const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
	SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
	SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
	while( seriesItr != seriesEnd ) {
		QString qserie = QString::fromStdString( seriesItr->c_str() ) ;
		_seriesDico.insert( qserie, QMap< QString,QString > () ) ;

		FileNamesContainer fileNames;


		ReaderType::Pointer reader = ReaderType::New();

		// Définition du dicom

		ImageIOType::Pointer dicomIO = ImageIOType::New();
		reader->SetImageIO(dicomIO);

		fileNames = nameGenerator->GetFileNames( seriesItr->c_str() ) ;
		reader->SetFileNames( fileNames );

		try {
			reader->Update();
		} catch ( itk::ExceptionObject & ex ) {
			qDebug() << ex.GetDescription() ;
			return ;
		}
		typedef itk::MetaDataDictionary DictionaryType ;
		const DictionaryType &dictionary = dicomIO->GetMetaDataDictionary();
		
		allTag( dictionary, _seriesDico[ qserie ] ) ;
		seriesItr++;
	}
}

QString DicomReader::getTag( const QString &entryId, const itk::MetaDataDictionary &dictionary ) {
	QString value = QObject::tr("undefined");

	itk::MetaDataObject<std::string>::ConstPointer entryValue = 0;
	const itk::MetaDataDictionary::ConstIterator tagItr = dictionary.Find(entryId.toStdString());

	if( tagItr != dictionary.End ()) {
		entryValue = dynamic_cast<itk::MetaDataObject<std::string> *> (tagItr->second.GetPointer());
		if ( entryValue ) {
			value = QString::fromStdString(entryValue->GetMetaDataObjectValue());
		}
	}
	return value;
}
		
void DicomReader::allTag( const itk::MetaDataDictionary &dictionary, QMap< QString,QString> & keyValues ) {
	std::vector< std::string > keys = dictionary.GetKeys() ;
	std::vector< std::string >::iterator keyIter = keys.begin() ;
	while ( keyIter != keys.end () )
	{
		const itk::MetaDataObjectBase * entryValue = dictionary.Get( *keyIter ) ;
		const itk::MetaDataObject<std::string> *readableEntryValue = dynamic_cast< const itk::MetaDataObject<std::string> *> (entryValue);
		if ( readableEntryValue )
		{
			QString value = QString::fromStdString( readableEntryValue->GetMetaDataObjectValue() );
			value = value.trimmed() ;
			if ( ! value.isEmpty() )
				keyValues.insert( QString::fromStdString(*keyIter), value ) ;
		}
		keyIter++ ;
	}
}
	
bool DicomReader::init_reading( QMap< QString, QMap< QString, QString > >::ConstIterator &filter, uint &height, uint &width, uint &depth, ReaderType::Pointer *reader ) {
	*reader = ReaderType::New();

	// Définition du dicom
	ImageIOType::Pointer dicomIO = ImageIOType::New();
	(*reader)->SetImageIO(dicomIO);

	// Mise en place de la lecture en serie
	NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();
	nameGenerator->SetUseSeriesDetails( true ); // to handle distinct series within a single repository
	nameGenerator->SetDirectory(_repository.toStdString());


	/// can we avoid the next lines ?
	const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
	SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
	SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
	while( seriesItr != seriesEnd ) {
		if ( filter.key().compare( seriesItr->c_str() ) == 0 ) break ;
		seriesItr++;
	}

	FileNamesContainer fileNames;
	fileNames = nameGenerator->GetFileNames( filter.key().toStdString() );
	(*reader)->SetFileNames( fileNames );


	/// following code valid if you only managed a single serie in the repository
	/// reader->SetFileNames(nameGenerator->GetInputFileNames());

	try	{
		(*reader)->Update();
	} catch( itk::ExceptionObject &ex ) {
		qDebug() << ex.GetDescription();
		return false;
	}

	// Dictionnaire
	const itk::MetaDataDictionary &dictionary = dicomIO->GetMetaDataDictionary();

	// Recherche de tag dans le fichier
	bool size_ok;

	height = getTag("0028|0010",dictionary).toUInt(&size_ok);
	if ( !size_ok )	{
		qWarning() << QObject::tr("Error : reading dimension (number of rows) 's image fails.");
		return false;
	}
	width = getTag("0028|0011",dictionary).toUInt(&size_ok);
	if ( !size_ok )	{
		qWarning() << QObject::tr("Error : reading dimension (number of cols) 's image fails.");
		return false;
	}
	depth = (*reader)->GetFileNames().size();
	return true ;
}

