#include "dicomreader.h"

#include "billon.h"

#include <QDebug>

#include <itkMetaDataDictionary.h>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>

namespace DicomReader
{

	// Déclaration de fonctions privées
	namespace
	{
        int enumerate_dicom_seriesWITHITK( const QString &repository, QMap< QString, QMap< QString,QString > > &seriesDico ) ;
        Billon* makeBillonFromDicomWithITK( const QString &repository, const std::string &filter );
		QString getTag( const QString &entryId, const itk::MetaDataDictionary &dictionary );
		void allTag( const itk::MetaDataDictionary &dictionary, QMap< QString, QString > & );
	}


    Billon* read( const QString &repository, const std::string &filter )
	{

        Billon* cube = makeBillonFromDicomWithITK( repository, filter );

		if ( cube == 0 )
		{
			qWarning() << QObject::tr("ERREUR : Impossible de lire le contenu du répertoire.");
		}

		return cube;
	}

    int enumerate_dicom_series( const QString &repository, QMap< QString, QMap< QString,QString > > &seriesDico ) {
        int num = enumerate_dicom_seriesWITHITK( repository, seriesDico ) ;
        if ( num == 0 ) {
            qWarning() << QObject::tr("ERREUR : Impossible de lire le contenu du répertoire.");
        }
        return num ;
    }

	// Implémentation des fonction privées
	namespace
	{

    inline int enumerate_dicom_seriesWITHITK( const QString &repository, QMap< QString, QMap< QString,QString > > &seriesDico ) {
        typedef itk::GDCMSeriesFileNames NamesGeneratorType;
        NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

        nameGenerator->SetUseSeriesDetails( true ); // to handle distinct series within a single repository

        nameGenerator->SetDirectory(repository.toStdString());

        typedef std::vector< std::string > SeriesIdContainer;
        const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
        SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
        SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
        while( seriesItr != seriesEnd ) {
            QString qserie = QString::fromStdString( seriesItr->c_str() ) ;
            seriesDico.insert( qserie, QMap< QString,QString > () ) ;

            typedef std::vector< std::string > FileNamesContainer;
            FileNamesContainer fileNames;


            // Définition des type de l'image
            const unsigned int InputDimension = 3;
            typedef int PixelType;
            typedef itk::Image<PixelType,InputDimension> ImageType;
            typedef itk::ImageSeriesReader<ImageType> ReaderType;
            ReaderType::Pointer reader = ReaderType::New();

            // Définition du dicom
            typedef itk::GDCMImageIO ImageIOType;
            ImageIOType::Pointer dicomIO = ImageIOType::New();
            reader->SetImageIO(dicomIO);

            fileNames = nameGenerator->GetFileNames( seriesItr->c_str() ) ;
            reader->SetFileNames( fileNames );

            try {
                reader->Update();
            } catch ( itk::ExceptionObject & ex ) {
                qDebug() << ex.GetDescription() ;
                return 0 ;
            }
            typedef itk::MetaDataDictionary DictionaryType ;
            const DictionaryType &dictionary = dicomIO->GetMetaDataDictionary();
            
            allTag( dictionary, seriesDico[ qserie ] ) ;
            seriesItr++;
        }
        return seriesDico.size() ;
    }

    inline
        Billon* makeBillonFromDicomWithITK( const QString &repository, const std::string & filter )
		{
            // Définition des type de l'image
			const unsigned int InputDimension = 3;
			typedef int PixelType; // INT à la base marchais bien.
			typedef itk::Image<PixelType,InputDimension> ImageType;
			typedef itk::ImageSeriesReader<ImageType> ReaderType;
			ReaderType::Pointer reader = ReaderType::New();

			// Définition du dicom
			typedef itk::GDCMImageIO ImageIOType;
			ImageIOType::Pointer dicomIO = ImageIOType::New();
			reader->SetImageIO(dicomIO);

			// Mise en place de la lecture en serie
            typedef itk::GDCMSeriesFileNames NamesGeneratorType;
			NamesGeneratorType::Pointer nameGenerator = NamesGeneratorType::New();

            nameGenerator->SetUseSeriesDetails( true ); // to handle distinct series within a single repository


            nameGenerator->SetDirectory(repository.toStdString());

            typedef std::vector< std::string >
            SeriesIdContainer;
            const SeriesIdContainer & seriesUID = nameGenerator->GetSeriesUIDs();
            SeriesIdContainer::const_iterator seriesItr = seriesUID.begin();
            SeriesIdContainer::const_iterator seriesEnd = seriesUID.end();
            while( seriesItr != seriesEnd )
            {
                if ( !filter.empty() ) {
                    if ( filter.compare( seriesItr->c_str() ) == 0 ) break ;
                }
                seriesItr++;

            }

            typedef std::vector< std::string > FileNamesContainer;
            FileNamesContainer fileNames;
            fileNames = nameGenerator->GetFileNames( filter );

            reader->SetFileNames( fileNames );


            /// following code valid if you only managed a single serie in the repository
            /// reader->SetFileNames(nameGenerator->GetInputFileNames());

			try
			{
				reader->Update();
			}
			catch( itk::ExceptionObject &ex )
			{
				qDebug() << ex.GetDescription();
				return 0;
			}

			// Dictionnaire
			const itk::MetaDataDictionary &dictionary = dicomIO->GetMetaDataDictionary();

			// Recherche de tag dans le fichier
			bool size_ok;

			const uint height = getTag("0028|0010",dictionary).toUInt(&size_ok);
			if ( !size_ok )
			{
				qWarning() << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.");
				return 0;
			}
			const uint width = getTag("0028|0011",dictionary).toUInt(&size_ok);
			if ( !size_ok )
			{
				qWarning() << QObject::tr("ERREUR : Lecture des dimensions de l'image impossible.");
				return 0;
			}
			const uint depth = reader->GetFileNames().size();

            // Création d'une matrice aux dimensions de l'image
			Billon * const billon = new Billon( height, width, depth );

			const ImageType::Pointer &image = reader->GetOutput();
			itk::ImageRegionConstIterator< ImageType > in( image,image->GetBufferedRegion() );
			int max, min;
			max = min = in.Value();
			for ( uint k=0; k<depth; k++ )
			{
				Slice &slice = billon->slice(k);
				for ( uint j=0; j<height; j++ )
				{
					for ( uint i=0; i<width; i++ )
					{
						const int &current = in.Value();
						slice.at(j,i) = current;
						max = qMax(max,current);
						min = qMin(min,current);
						++in;
					}
				}
			}

			billon->setMaxValue(max);
			billon->setMinValue(min);

			const ImageType::SpacingType &spacing = image->GetSpacing();
			billon->setVoxelSize(spacing[0],spacing[1],spacing[2]);
			return billon;
		}

		inline
		QString getTag( const QString &entryId, const itk::MetaDataDictionary &dictionary )
		{
			QString value = QObject::tr("indéfinie");

			itk::MetaDataObject<std::string>::ConstPointer entryValue = 0;
			const itk::MetaDataDictionary::ConstIterator tagItr = dictionary.Find(entryId.toStdString());

			if( tagItr != dictionary.End ())
			{
				entryValue = dynamic_cast<itk::MetaDataObject<std::string> *> (tagItr->second.GetPointer());
				if ( entryValue )
				{
					value = QString::fromStdString(entryValue->GetMetaDataObjectValue());
				}
			}
			return value;
		}
		
		inline
		void allTag( const itk::MetaDataDictionary &dictionary, QMap< QString,QString> & keyValues )
		{
			
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
	}

}

