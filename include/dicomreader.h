#ifndef DICOMREADER_H
#define DICOMREADER_H

#include "def_billon.h"
#include <vector>
#include <string>
class QString;
#include <QMap>

namespace DicomReader
{
    int enumerate_dicom_series( const QString &repository, QMap< QString, QMap< QString,QString > > &seriesDico ) ;
    Billon* read( const QString &repository, const std::string &filter );
}

#endif // DICOMREADER_H
