#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include "define.h"
#include "def_billon.h"
#include "def_coordinate.h"

#include <QtGlobal>
#include <QColor>

template<typename T> class Interval;
class QImage;

class SliceView
{

public:
	explicit SliceView();

    void drawSlice( QImage &image, const Billon &billon, const uint &sliceNumber, const Interval<int> &intensityInterval, const int v );
    void drawOverSlice( QImage &image, const BillonTpl<char> &billon, const uint &sliceNumber,int x_shift, int y_shift, int z_shift, bool content, const QRgb color );

private :
    void drawCurrentSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval, const int v );
    void drawOverCurrentSlice( QImage &image, const BillonTpl<char> &billon, const uint &sliceIndex,int x_shift, int y_shift, int z_shift, bool content, const QRgb color );
private:

};

#endif // SLICEVIEW_H
