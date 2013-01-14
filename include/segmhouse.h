#ifndef SEGMHOUSE_H
#define SEGMHOUSE_H

#include <def_billon.h>

#include "DGtal/shapes/Shapes.h"
#include "DGtal/helpers/StdDefs.h"
#include <QString>
#include "interval.h"
class SegmHouse
{
public:
    SegmHouse( Billon *dicomSerie);
    ~SegmHouse() ;
    Z3i::DigitalSet * shape() const {
        return _pShapeSet ;
    }
    const QString name() const {
        return _name ;
    }
 protected:
    Billon *_rDicomSerie ;
    Z3i::DigitalSet *_pShapeSet ;
    QString _name ;
private:

};

#endif // SEGMHOUSE_H
