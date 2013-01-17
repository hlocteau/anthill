#include <connexcomponentextractor.h>

#include <billon.h>


template<typename IN, typename OUT>
BillonTpl<OUT> * ConnexComponentExtractor<IN,OUT>::run( const BillonTpl<IN> & billon, const int minimumSize, const IN threshold ) {
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;

	int nbLabel = 0;
	QMap<int, QList<iCoord3D> > connexComponentList;
	arma::Mat<OUT>* labels = new arma::Mat<OUT>(height, width);
	arma::Mat<OUT>* oldSlice = new arma::Mat<OUT>(height, width);
	oldSlice->fill(0);
	arma::Mat<OUT>* tmp;

	//On parcours les tranches 1 par 1
	for ( unsigned int k=0 ; k<depth ; ++k )
	{
		nbLabel = twoPassAlgorithm((*oldSlice),billon.slice(k),(*labels),connexComponentList,k,nbLabel,threshold);
		tmp = oldSlice;
		oldSlice = labels;
		labels = tmp;
		tmp = 0;
	}

	delete labels;
	delete oldSlice;

	BillonTpl<OUT>* components = new BillonTpl<OUT>(billon.n_rows,billon.n_cols,billon.n_slices);
	components->fill(0);

	QMap<int, QList<iCoord3D> >::ConstIterator iterComponents;
	QList<iCoord3D>::ConstIterator iterCoords;
	int counter = 1;
	for ( iterComponents = connexComponentList.constBegin() ; iterComponents != connexComponentList.constEnd() ; ++iterComponents )
	{
		if ( iterComponents.value().size() > minimumSize )
		{
			for ( iterCoords = iterComponents.value().constBegin() ; iterCoords != iterComponents.value().constEnd() ; ++iterCoords )
			{
				components->at((*iterCoords).y,(*iterCoords).x,(*iterCoords).z) = counter;
			}
			++counter;
		}
	}
	qDebug() << QObject::tr("Nombre de composantes = %1").arg(counter-1);
	components->setMinValue(0);
	components->setMaxValue(counter-1);

	return components;
}

template<typename IN, typename OUT>
arma::Mat<OUT> * ConnexComponentExtractor<IN,OUT>::run( const arma::Mat<IN> & slice, const int minimumSize, const IN threshold ) {
	const uint width = slice.n_cols;
	const uint height = slice.n_rows;

	QMap<int, QList<iCoord2D> > connexComponentList;
	QMap<int, int> tableEquiv;
	QList<int> voisinage;
	uint j, i;
	int mini, nbLabel, label, currentEquiv;

	arma::Mat<OUT> labels(height, width);
	labels.fill(0);
	nbLabel = 0;
	//On parcourt une première fois la tranche
	for ( j=1 ; j<height ; ++j)
	{
		for ( i=1 ; i<width-1 ; ++i)
		{
			//Si on a un voxel
			if ( slice.at(j,i) > threshold )
			{
				//On sauvegarde la valeur des voisins non nuls
				voisinage.clear();
				//Voisinage de la face courante
				if (labels.at(j,i-1)) voisinage.append(labels.at(j,i-1));
				if (labels.at(j-1,i-1)) voisinage.append(labels.at(j-1,i-1));
				if (labels.at(j-1,i)) voisinage.append(labels.at(j-1,i));
				if (labels.at(j-1,i+1)) voisinage.append(labels.at(j-1,i+1));
				//Si ses voisins n'ont pas d'étiquette
				if ( voisinage.isEmpty() )
				{
					++nbLabel;
					labels.at(j,i) = nbLabel;
				}
				//Si ses voisins ont une étiquette
				else if ( voisinage.size() == 1 )
				{
					labels.at(j,i) = voisinage[0];
				}
				else
				{
					QList<int>::ConstIterator iterVoisin = voisinage.constBegin();
					mini = (*iterVoisin++);
					while ( iterVoisin != voisinage.constEnd() )
					{
						mini = qMin(mini,(*iterVoisin++));
					}
					labels.at(j,i) = mini;
					for ( iterVoisin = voisinage.constBegin() ; iterVoisin != voisinage.constEnd() ; ++iterVoisin )
					{
						if ( (*iterVoisin) > mini )
						{
							if ( tableEquiv.contains(*iterVoisin) )
							{
								currentEquiv = tableEquiv[*iterVoisin];
								if ( mini > currentEquiv )
								{
									tableEquiv[*iterVoisin] = mini;
									while (tableEquiv.contains(mini))
									{
										mini = tableEquiv[mini];
									}
									if ( currentEquiv < mini )
									{
										tableEquiv[mini] = currentEquiv;
										labels.at(j,i) = currentEquiv;
									}
									else if ( currentEquiv > mini )
									{
										tableEquiv[currentEquiv] = mini;
										labels.at(j,i) = mini;
									}
								}
								else if ( mini < currentEquiv )
								{
									tableEquiv[*iterVoisin] = currentEquiv;
									while (tableEquiv.contains(currentEquiv))
									{
										currentEquiv = tableEquiv[currentEquiv];
									}
									if ( currentEquiv > mini )
									{
										tableEquiv[currentEquiv] = mini;
										labels.at(j,i) = mini;
									}
									else if ( currentEquiv < mini )
									{
										tableEquiv[mini] = currentEquiv;
										labels.at(j,i) = currentEquiv;
									}
								}
							}
							else
							{
								tableEquiv[*iterVoisin] = mini;
							}
						}
					}
				}
			}
		}
	}

	//Résolution des chaines dans la table d'équivalence
	QMap<int, int>::ConstIterator iterTable;
	int value;
	for ( iterTable = tableEquiv.constBegin() ; iterTable != tableEquiv.constEnd() ; ++iterTable )
	{
		value = iterTable.value();
		while (tableEquiv.contains(value))
		{
			value = tableEquiv[value];
		}
		tableEquiv[iterTable.key()] = value;
	}
	for ( j=0 ; j<height ; ++j )
	{
		for ( i=0 ; i<width ; ++i )
		{
			label = labels.at(j,i);
			//Si on a un voxel
			if (label)
			{
				if (tableEquiv.contains(label))
				{
					labels.at(j,i) = tableEquiv[label];
					label = labels.at(j,i);
				}
				if (!connexComponentList.contains(label)) connexComponentList[label] = QList<iCoord2D>();
				connexComponentList[label].append(iCoord2D(i,j));
			}
		}
	}

	arma::Mat<OUT> *bigestComponentsInSlice = new arma::Mat<OUT>(width,height);
	bigestComponentsInSlice->fill(0);
	QMap<int, QList<iCoord2D> >::ConstIterator iterComponents;
	QList<iCoord2D>::ConstIterator iterCoords;
	int counter = 1;
	for ( iterComponents = connexComponentList.constBegin() ; iterComponents != connexComponentList.constEnd() ; ++iterComponents )
	{
		if ( iterComponents.value().size() > minimumSize )
		{
			for ( iterCoords = iterComponents.value().constBegin() ; iterCoords != iterComponents.value().constEnd() ; ++ iterCoords )
			{
				bigestComponentsInSlice->at((*iterCoords).y,(*iterCoords).x) = counter;
			}
			++counter;
		}
	}

	return bigestComponentsInSlice;
}

template<typename IN, typename OUT>
int ConnexComponentExtractor<IN,OUT>::twoPassAlgorithm( const arma::Mat<OUT> &oldSlice, const arma::Mat<IN> &currentSlice, arma::Mat<OUT> &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel, const IN threshold )
{
	const uint width = currentSlice.n_cols;
	const uint height = currentSlice.n_rows;

	QMap<int, int> tableEquiv;
	QList<int> voisinage;
	uint j, i;
	int mini, oldStart, sup, inf, label;
	bool isOld;
	labels.fill(0);
	//On parcourt une première fois la tranche
	for ( j=1 ; j<height-1 ; ++j)
	{
		for ( i=1 ; i<width-1 ; ++i)
		{
			//Si on a un voxel
			if ( currentSlice.at(j,i) > threshold )
			{
				//On sauvegarde la valeur des voisins non nuls
				voisinage.clear();
				//Voisinage de la face courante
				if (labels.at(j,i-1)) voisinage.append(labels.at(j,i-1));
				if (labels.at(j-1,i-1))voisinage.append(labels.at(j-1,i-1));
				if (labels.at(j-1,i)) voisinage.append(labels.at(j-1,i));
				if (labels.at(j-1,i+1)) voisinage.append(labels.at(j-1,i+1));
				oldStart = voisinage.size();
				//Voisinage de la face arrière
				if (oldSlice.at(j-1,i-1)) voisinage.append(oldSlice.at(j-1,i-1));
				if (oldSlice.at(j-1,i)) voisinage.append(oldSlice.at(j-1,i));
				if (oldSlice.at(j-1,i+1)) voisinage.append(oldSlice.at(j-1,i+1));
				if (oldSlice.at(j,i-1)) voisinage.append(oldSlice.at(j,i-1));
				if (oldSlice.at(j,i)) voisinage.append(oldSlice.at(j,i));
				if (oldSlice.at(j,i+1)) voisinage.append(oldSlice.at(j,i+1));
				if (oldSlice.at(j+1,i-1)) voisinage.append(oldSlice.at(j+1,i-1));
				if (oldSlice.at(j+1,i)) voisinage.append(oldSlice.at(j+1,i));
				if (oldSlice.at(j+1,i+1)) voisinage.append(oldSlice.at(j+1,i+1));
				//Si ses voisins n'ont pas d'étiquette
				if ( voisinage.isEmpty() )
				{
					++nbLabel;
					labels.at(j,i) = nbLabel;
				}
				//Si ses voisins ont une étiquette
				else
				{
					QList<int>::ConstIterator iterVoisin = voisinage.constBegin();
					mini = (*iterVoisin++);
					while ( iterVoisin != voisinage.constEnd() )
					{
						mini = qMin(mini,(*iterVoisin++));
					}
					//Attribution de la valeur minimale au voxel courant
					labels.at(j,i) = mini;
					isOld = connexComponentList.contains(mini);
					//Mise à jour de la table d'équivalence pour la face courante
					//et fusion des liste de sommets si un voxel fusionne des composantes connexes antérieures
					for ( int ind=0 ; ind<oldStart ; ++ind )
					{
						if ( voisinage[ind] > mini )
						{
							tableEquiv[voisinage[ind]] = mini;
							if (isOld && connexComponentList.contains(voisinage[ind]))
							{
								connexComponentList[mini].append(connexComponentList.take(voisinage[ind]));
							}
						}
					}
					if ( isOld )
					{
						for ( int ind=oldStart ; ind<voisinage.size() ; ++ind )
						{
							if ( voisinage[ind] != mini )
							{
								if ( mini>voisinage[ind] )
								{
									sup = mini;
									inf = voisinage[ind];
								}
								else
								{
									sup = voisinage[ind];
									inf = mini;
								}
								tableEquiv[sup] = inf;
								connexComponentList[inf].append(connexComponentList.take(sup));
							}
						}
					}
				}
			}
		}
	}
	//Résolution des chaines dans la table d'équivalence
	QMap<int, int>::ConstIterator iterTable;
	int value;
	for ( iterTable = tableEquiv.constBegin() ; iterTable != tableEquiv.constEnd() ; ++iterTable )
	{
		value = iterTable.value();
		while (tableEquiv.contains(value))
		{
			value = tableEquiv[value];
		}
		tableEquiv[iterTable.key()] = value;
	}
	for ( j=0 ; j<height ; ++j )
	{
		for ( i=0 ; i<width ; ++i )
		{
			label = labels.at(j,i);
			//Si on a un voxel
			if (label)
			{
				if (tableEquiv.contains(label))
				{
					labels.at(j,i) = tableEquiv[label];
					label = labels.at(j,i);
				}
				if (!connexComponentList.contains(label)) connexComponentList[label] = QList<iCoord3D>();
				connexComponentList[label].append(iCoord3D(i,j,k));
			}
		}
	}
	return nbLabel;
}
