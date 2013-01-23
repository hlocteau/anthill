#include <io/AntHillFile.hpp>
#include <QXmlStreamWriter>
#include <QFile>
#include <iostream>

AntHillFile::AntHillFile( const QString &filename, const QString &dicomfolder, const QString &uid, const QMap< QString, QString > &dictionary ) {
	_error = false ;
	QFile qfile( filename ) ;
	if( !qfile.open(QFile::WriteOnly) ) {
		std::cerr << "Error : aiming at writing file "<<filename.toStdString()<<", the operation fails." << std::endl;
		_error = true ;
		return ;
	}
	QXmlStreamWriter stream(&qfile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
	stream.writeStartElement("AntHillProject");
	
	stream.writeStartElement("Dicom");
	stream.writeAttribute("folder", dicomfolder );
	stream.writeAttribute("serie-uid", uid );
	stream.writeEndElement(); // Dicom
	stream.writeStartElement("DicomMetaData");
	
	for ( QMap< QString, QString >::ConstIterator iter = dictionary.begin() ; iter != dictionary.end() ; iter++ ) {
		stream.writeStartElement("Info");
		stream.writeAttribute("name", iter.key() );
		stream.writeAttribute("value",  iter.value() );
		stream.writeEndElement(); // info
	}
	stream.writeEndElement(); // DicomMetaData
	stream.writeEndElement(); // AntHillProject
	stream.writeEndDocument(); 
	qfile.close() ;
}
