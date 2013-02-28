#ifndef DICOMREADER_H
#define DICOMREADER_H

#include "def_billon.h"
#include "billon.h"

#include <vector>
#include <string>
class QString;
#include <QMap>

#include <itkMetaDataDictionary.h>
#include <itkImage.h>
#include <itkImageSeriesReader.h>
#include <itkGDCMImageIO.h>
#include <itkGDCMSeriesFileNames.h>
#include <QDebug>


class DicomReader {
	public :
		static const int InputDimension = 3 ;
		typedef int                                  PixelType;
		typedef itk::Image<PixelType,InputDimension> ImageType;
		typedef itk::ImageSeriesReader<ImageType>    ReaderType;
		typedef itk::GDCMImageIO                     ImageIOType;
		typedef itk::GDCMSeriesFileNames             NamesGeneratorType;
		typedef std::vector< std::string >           SeriesIdContainer;
		typedef std::vector< std::string >           FileNamesContainer;
		
		DicomReader( const QString &repository ) ;
		
		/**
		 * \return an iterator pointing to the first serie and its corresponding dictionary
		 */
		QMap< QString, QMap< QString, QString > >::ConstIterator begin() const {
			return _seriesDico.begin() ;
		}
		/**
		 * \return an iterator pointing to the past-the-end serie
		 */
		QMap< QString, QMap< QString, QString > >::ConstIterator end() const {
			return _seriesDico.end() ;
		}
		
		template <typename T> 
		BillonTpl<T>* get                     ( QMap< QString, QMap< QString, QString > >::ConstIterator &filter );
	protected:
		void          enumerate_dicom_series  ( ) ;
		bool          init_reading            ( QMap< QString, QMap< QString, QString > >::ConstIterator &filter, uint &height, uint &width, uint &depth, ReaderType::Pointer *reader ) ;
		QString       getTag                  ( const QString &entryId, const itk::MetaDataDictionary &dictionary ) ;
		void          allTag                  ( const itk::MetaDataDictionary &dictionary, QMap< QString,QString> & keyValues ) ;
    private:
		QString                                  _repository ;
		QMap< QString, QMap< QString,QString > > _seriesDico ;
} ;

template <typename T> BillonTpl<T>* DicomReader::get( QMap< QString, QMap< QString, QString > >::ConstIterator & filter ) {
	ReaderType::Pointer reader ;
	uint height, width, depth ;
	if ( !init_reading( filter, height, width, depth, &reader ) ) {
		return 0 ;
	}

	// Création d'une matrice aux dimensions de l'image
	BillonTpl<T> * billon = new BillonTpl<T>( height, width, depth );

	const ImageType::Pointer &image = reader->GetOutput();
	itk::ImageRegionConstIterator< ImageType > in( image,image->GetBufferedRegion() );
	int max, min;
	max = min = in.Value();
	for ( uint k=0; k<depth; k++ ) {
		arma::Mat<T> &slice = billon->slice(k);
		for ( uint j=0; j<height; j++ )	{
			for ( uint i=0; i<width; i++ ) {
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

#endif // DICOMREADER_H
