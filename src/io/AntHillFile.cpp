#include <io/AntHillFile.hpp>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <iostream>

bool AntHillFile::save( const QString &filename ) {
	QFile qfile( filename ) ;
	if( !qfile.open(QFile::WriteOnly) ) {
		std::cerr << "Error : aiming at writing file "<<filename.toStdString()<<", the operation fails." << std::endl;
		_error = true ;
		return !_error ;
	}
	QXmlStreamWriter stream(&qfile);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
	stream.writeStartElement("AntHillProject");
	
	stream.writeStartElement("Dicom");
	stream.writeAttribute("folder", _dicomFolder );
	stream.writeAttribute("serie-uid", _dicomUID );
	stream.writeEndElement(); // Dicom
	
	stream.writeStartElement("DicomMetaData");
	
	for ( QMap< QString, QString >::ConstIterator iter = _dictionary.begin() ; iter != _dictionary.end() ; iter++ ) {
		stream.writeStartElement("Info");
		stream.writeAttribute("name", iter.key() );
		stream.writeAttribute("value",  iter.value() );
		stream.writeEndElement(); // info
	}
	stream.writeEndElement(); // DicomMetaData
	
	if ( !_process.isEmpty() ) {
		stream.writeStartElement("Process");
		for ( QMap< QString, QMap< QString,QString > >::ConstIterator iter = _process.begin() ; iter != _process.end() ; iter++ ) {
			stream.writeStartElement("Algorithm");
			stream.writeAttribute("name", iter.key() );
			for ( QMap< QString, QString >::ConstIterator param = iter.value().begin() ; param != iter.value().end() ; param++ ) {
				stream.writeStartElement("param");
				stream.writeAttribute("name", param.key() );
				stream.writeAttribute("value",  param.value() );
				stream.writeEndElement(); // param
			}
			stream.writeEndElement(); // Algorithm
		}
		stream.writeEndElement(); // Process
	}
	
	stream.writeEndElement(); // AntHillProject	
	stream.writeEndDocument(); 
	qfile.close() ;
	return !_error ;
}

bool AntHillFile::addProcess( const QString &name, const QMap< QString, QString > &details ) {
	if ( _process.contains( name ) ) {
		_process[ name ].clear() ;
		_process[ name ] = details ;
		return true ;
	}
	
	_process.insert( name, details ) ;
	
	return true ;
}

bool AntHillFile::load( const QString &filename ) {
	QFile qfile( filename ) ;
	if( !qfile.open(QFile::ReadOnly) ) {
		std::cerr << "Error : aiming at reading file "<<filename.toStdString()<<", the operation fails." << std::endl;
		_error = true ;
		return !_error ;
	}
	QXmlStreamReader stream( &qfile ) ;
	QString algname ="";
	while ( !stream.atEnd() ) {
		stream.readNext() ;
		if ( stream.isStartElement() ) {
			if ( stream.name() == "Info" ) {
				QXmlStreamAttributes attr = stream.attributes() ;
				_dictionary.insert( attr.value( "name" ).toString() , attr.value( "value" ).toString()  ) ;
			} else if ( stream.name() == "Dicom" ) {
				QXmlStreamAttributes attr = stream.attributes() ;
				_dicomFolder = attr.value( "folder" ).toString();
				_dicomUID = attr.value( "serie-uid" ).toString();
			} else if ( stream.name() == "Algorithm" ) {
				QXmlStreamAttributes attr = stream.attributes() ;
				algname = attr.value( "name" ).toString() ;
				_process.insert( algname , QMap< QString,QString> ()  ) ;
			} else if ( stream.name() == "param" ) {
				if( algname.isEmpty() ) {
					_error = true ;
					qfile.close() ;
					return !_error ;
				}
				QXmlStreamAttributes attr = stream.attributes() ;
				_process[ algname ].insert( attr.value( "name" ).toString() , attr.value( "value" ).toString()  ) ;
			}
		}
	}
	_error = stream.hasError() ;
	qfile.close() ;
	return !_error ;
}
