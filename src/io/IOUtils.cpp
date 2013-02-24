#include <io/IOUtils.h>
#include <connexcomponentextractor.h>

BillonTpl< arma::u8 > * load_maincc( QString filename ) {
	Pgm3dFactory< arma::u8 > factory ;
	BillonTpl< arma::u8 > *pimg = factory.read( filename ) ;
	
	/// keep only the biggest connected component
	ConnexComponentExtractor< arma::u8,arma::u32 > cce ;
	BillonTpl< arma::u32 > *lblImg = cce.run( *pimg ) ;
	ConnexComponentExtractor< arma::u8,arma::u32 >::TMapVolume::ConstIterator iterVolume = cce.volumes().begin(),
																			  iterVolumeEnd = cce.volumes().end(),
																			  mainVolume ;
	for ( mainVolume = iterVolume ; iterVolume != iterVolumeEnd ; iterVolume++ )
		if ( mainVolume.value() < iterVolume.value() )
			mainVolume = iterVolume ;
	BillonTpl< arma::u32 >::const_iterator iterLbl = lblImg->begin(),
										   iterLblEnd = lblImg->end() ;
	BillonTpl< arma::u8 >::iterator        iterSkel = pimg->begin();
	for ( ; iterLbl != iterLblEnd ; iterLbl++,iterSkel++ )
		*iterSkel = ( *iterLbl == mainVolume.key() ? 1 : 0 ) ;
	delete lblImg ;
	return pimg ;
}
