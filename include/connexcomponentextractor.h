#ifndef CONNEX_COMPONENT_EXTRACTOR_H
#define CONNEX_COMPONENT_EXTRACTOR_H

#include <def_billon.h>
#include <def_coordinate.h>
#include <coordinate.h>
#include <QMap>
#include <limits>
/**
 * \note the 26-connexity is used
 * \todo add connexity as a parameter
 */
template <typename IN, typename OUT>
class ConnexComponentExtractor {
	public:
		typedef OUT				value_type ;
		typedef QMap<OUT,int>	TMapVolume ;
		typedef QMap<OUT, std::pair<iCoord3D,iCoord3D> > TMap3DBounds ;
		typedef QMap<OUT, std::pair<iCoord2D,iCoord2D> > TMap2DBounds ;

		BillonTpl<OUT> * run( const BillonTpl<IN> &billon, const int minimumSize=0, const IN threshold=0 );
		arma::Mat<OUT> * run( const arma::Mat<IN> &slice, const int minimumSize=0, const IN threshold=0 );
		
		const TMapVolume & volumes() const {
			return _volumes ;
		}
		const TMap3DBounds & bounds3D() const {
			return _bounds3D ;
		}
		const TMap3DBounds & bounds2D() const {
			return _bounds2D ;
		}
		
	private:
		TMapVolume _volumes ;
		TMap3DBounds _bounds3D ;
		TMap2DBounds _bounds2D ;
		/**
		 * \fn		int twoPassAlgorithm( Slice &oldSlice, Slice &currentSlice, Slice &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel )
		 * \brief	Algorithme de labelisation de composantes connexes en 3 dimensions
		 * \param	oldSlice Tranche précédente déjà labelisée
		 * \param	currentSlice Tranche courante où sont extraites les composantes connexes
		 * \param	labels Matrice 2D qui contiendra la tranche courante labelisée
		 * \param	connexComponentList Liste qui associe à un label la liste des coordonnées des points de sa composante connexe
		 * \param	k Numéro de la tranche courante dans l'image globale
		 * \param	nbLabel Nombre de labels déjà attribués
		 * \return	le dernier label attribué
		 */
		int32_t twoPassAlgorithm( const arma::Mat<int32_t> &oldSlice, const arma::Mat<IN> &currentSlice, 
								arma::Mat<int32_t> &labels, QMap<int32_t, QList<iCoord3D> > &connexComponentList, int k, int32_t nbLabel, const IN threshold );
} ;

template<typename T>
bool have_touching_conn_comp( const BillonTpl< T > & Label ) ;


#include <connexcomponentextractor.ih>

#endif // CONNEX_COMPONENT_EXTRACTOR_H
