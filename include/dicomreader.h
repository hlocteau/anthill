#ifndef DICOMREADER_H
#define DICOMREADER_H

#include "def_billon.h"
#include <vector>
#include <string>
class QString;

namespace DicomReader
{
    int enumerate_dicom_series( const QString &repository, std::vector< std::string > &series ) ;
    Billon* read( const QString &repository, const std::string &filter );
}

#endif // DICOMREADER_H
