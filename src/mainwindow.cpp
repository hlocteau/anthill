#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "billon.h"
#include "coordinate.h"
#include "dicomreader.h"

#include "geom2d.h"

#include <QFileDialog>
#include <QMouseEvent>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "DGtal/images/ImageSelector.h"
#include "DGtal/io/writers/PNMWriter.h"
#include "DGtal/io/colormaps/GrayscaleColorMap.h"

bool USE_SEGM_TOOL = true ;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    _ui(new Ui::MainWindow)
{
    _currentSlice = 0;
    _currentSerie = -1 ;
    _ui->setupUi(this);
    _bViewSegm = false ;
    _ui->_labelSliceView->installEventFilter(this);
    //_ui->_labelSliceView->installEventFilter(&_sliceZoomer);

    _ui->sequenceSlider->setEnabled(false);
    _ui->spinMinIntensity->setEnabled(false);
    _ui->spinMaxIntensity->setEnabled(false);

    _billon = 0 ;
    _segmImg = 0 ;
    _skelImg = 0 ;
    _zoomFactor = 1 ;
}

MainWindow::~MainWindow()
{
    on_actionClose_folder_triggered() ;
    delete _ui;
}

void MainWindow::on_actionClose_folder_triggered()
{
    _currentRepository.clear() ;
    _seriesUID.clear() ;
    _shortDescriptionOfSeries.clear() ;

    _ui->sequenceSlider->setSliderPosition(0);
    _ui->spinMinIntensity->setValue(0);
    _ui->spinMaxIntensity->setValue(0);
    _ui->sequenceSlider->setEnabled(false);
    _ui->spinMinIntensity->setEnabled(false);
    _ui->spinMaxIntensity->setEnabled(false);

    update_list_of_series() ;
    closeImage();
}


void MainWindow::on_actionExit_triggered()
{
   close();
}

void MainWindow::update_list_of_series()
{

    _ui->listWidget->clear();
    if ( _currentRepository.isNull() ) {

        return ;
    }
    std::vector< std::map< QString, QString > >::iterator seriesDicoIt = _shortDescriptionOfSeries.begin() ;
    for ( std::vector< std::string >::iterator serieIt = _seriesUID.begin() ;
            serieIt != _seriesUID.end() ; serieIt++,seriesDicoIt++ )
    {
        _ui->listWidget->addItem( serieIt->c_str() );
    }

}

void MainWindow::on_actionOpen_folder_triggered()
{

    QString folderName = QFileDialog::getExistingDirectory(0,tr("select DICOM folder"),QDir::homePath(),QFileDialog::ShowDirsOnly);
    if ( !folderName.isEmpty() )
    {
        closeImage();
        DicomReader::enumerate_dicom_series( folderName, _seriesUID ) ;
        _currentRepository = folderName ;
        update_list_of_series();
    }

}

void MainWindow::closeImage() {

    if ( _billon != 0 )
    {
        delete _billon;
        _billon = 0;
    }
    if ( _segmImg != 0 )
      {
        delete _segmImg;
        _segmImg = 0 ;
      }
    if ( _skelImg != 0 )
      {
        delete _skelImg;
        _skelImg = 0 ;
      }
    _ui->segmLoadButton->setEnabled(false);
    _ui->x_shift->setEnabled(false);
    _ui->y_shift->setEnabled(false);
    _ui->z_shift->setEnabled(false);
    _ui->contentCheckBox->setEnabled(false);
    _ui->segmCheckBox->setEnabled(false);

    _ui->skelLoadButton->setEnabled(false);
    _ui->x_shift_skel->setEnabled(false);
    _ui->y_shift_skel->setEnabled(false);
    _ui->z_shift_skel->setEnabled(false);
    _ui->contentSkelCheckBox->setEnabled(false);
    _ui->skelCheckBox->setEnabled(false);

    _zoomFactor = 1 ;
    _mainPix = QImage(0,0,QImage::Format_ARGB32);
    //_segmPix = QImage(0,0,QImage::Format_ARGB32);
    drawSlice();

}


void export_binary_slice_as_pgm( const Slice &slice, const Interval<int> &range, int th, const std::string &filename )
{
    typedef DGtal::GrayscaleColorMap<unsigned char>                         Gray;
    typedef DGtal::ImageSelector< DGtal::Z2i::Domain, unsigned char>::Type  GrayImage;

    DGtal::Z2i::Point   pBR ( 1, 1);
    DGtal::Z2i::Point   pUL ( slice.n_cols, slice.n_rows );
    GrayImage           image(DGtal::Z2i::Domain(pBR,pUL));
    if ( range.width() == 0 )
    {
        for ( DGtal::Z2i::Domain::ConstIterator pt2D = image.domain().begin() ; pt2D != image.domain().end() ; pt2D++ )
            if ( slice.at((*pt2D).at(1),(*pt2D).at(0)) >= range.min() )
                image.setValue( *pt2D, 255 ) ;
    }
    else
    {
        for ( DGtal::Z2i::Domain::ConstIterator pt2D = image.domain().begin() ; pt2D != image.domain().end() ; pt2D++ )
        {
            int value = slice.at((*pt2D).at(1),(*pt2D).at(0)) ;
            if (  range.containsClosed( value ) ) {
                if ( ( ( ( value - range.min() ) * 255 ) / range.size() ) >= th )
                    image.setValue( *pt2D, 255 ) ;
            }
        }

    }
    PNMWriter<GrayImage,Gray>::exportPGM( filename,image,0,255);
}

void MainWindow::drawSlice( bool newContent ){

    if ( _billon != 0 )	{
        if ( newContent ) {
            Interval<int> range_img (_ui->spinMinIntensity->value(),_ui->spinMaxIntensity->value());
            _mainPix.fill(0xff0000CC);
            if ( !_bViewSegm )
                _sliceView->drawSlice(_mainPix,*_billon,_currentSlice, range_img, range_img.max());
            else
                _sliceView->drawSlice(_mainPix,*_billon,_currentSlice, range_img, _ui->binSpinBox->value());


            if ( _segmImg && _ui->segmCheckBox->isChecked() ) {
              _sliceView->drawOverSlice(_mainPix,*_segmImg, _currentSlice, _ui->x_shift->value(), _ui->y_shift->value(),_ui->z_shift->value(), _ui->contentCheckBox->isChecked(), qRgb(255,255,0));
            }

            if ( _skelImg && _ui->skelCheckBox->isChecked() ) {
              _sliceView->drawOverSlice(_mainPix,*_skelImg, _currentSlice, _ui->x_shift_skel->value(), _ui->y_shift_skel->value(),_ui->z_shift_skel->value(), _ui->contentSkelCheckBox->isChecked(),qRgb(0,0,255));
            }
        }
    }else{
        _ui->SlicePosition->setText(tr("Position"));
        _mainPix = QImage(1,1,QImage::Format_ARGB32);
    }
    _ui->_labelSliceView->setPixmap( QPixmap::fromImage(_mainPix).scaled(_mainPix.width()*_zoomFactor,_mainPix.height()*_zoomFactor,Qt::KeepAspectRatio) );
}

void helpmetounderstand() {
    DGtal::Z3i::Point p1( -50, -50, -50 );
    DGtal::Z3i::Point p2( 50, 50, 50 );
    DGtal::Z3i::Domain domain( p1, p2 );
    DGtal::Z3i::Point c( 0, 0 );
    // diamond of radius 30
    DGtal::Z3i::DigitalSet diamond_set( domain );
    for ( DGtal::Z3i::Domain::ConstIterator it = domain.begin(); it != domain.end(); ++it )
    {
     if ( (*it - c ).norm1() <= 5 ) diamond_set.insertNew( *it );
    }
    DGtal::Z3i::Object6_18 diamond( DGtal::Z3i::dt6_18, diamond_set );
    // The following line takes almost no time.
    DGtal::Z3i::Object6_18 diamond_clone( diamond );
    // Since one of the objects is modified, the set is duplicated at the following line
    diamond_clone.pointSet().erase( c );
    DGtal::Z3i::Object6_18 bdiamond = diamond.border(); // one component
    DGtal::Z3i::Object6_18 bdiamond_clone = diamond_clone.border(); // two components
    std::cout<<"diamond is"<<std::endl;
    for ( DGtal::Z3i::DigitalSet::Iterator pt = diamond.begin() ; pt != diamond.end() ; pt++ ) std::cout<<(*pt)<<" ";
    std::cout<<std::endl<<std::endl;

    std::cout<<"diamond's boundary is"<<std::endl;
    for ( DGtal::Z3i::DigitalSet::Iterator pt = bdiamond.begin() ; pt != bdiamond.end() ; pt++ ) std::cout<<(*pt)<<" ";
    std::cout<<std::endl<<std::endl;
    std::cout<<"diamond : "<<diamond.size()<<" boundary : "<<bdiamond.size()<<" twin boundary : "<<bdiamond_clone.size()<<std::endl;
}

std::string set_pgmfoldername( const std::vector< std::string > &dico ) {
    assert( !dico.empty() ) ;

    if ( dico.size() == 1 )
        return dico[0] ;

    /// identify the largest prefix shared by the distinct series' entry
    uint pos = 0 ;
    uint entry ;
    while ( pos != dico[0].size() )
    {
        for ( entry = 1 ; entry != dico.size() ; entry++ )
            if ( dico[ entry ][ pos ] != dico[0][pos] ) break ;
        if ( entry != dico.size() )
            break ;
        pos++ ;
    }
    return dico[0].substr( 0, pos ) ;
}

void export_segmentation( const std::vector< std::string > &dico, const Billon *data, uint id, const Interval<int> &range, int th) {
    std::string pgmfoldername = set_pgmfoldername( dico ) ;
    boost::filesystem::path pgmfolderpath = QDir::homePath().toStdString() ;
    pgmfolderpath /= "outputData";
    pgmfolderpath /= pgmfoldername + "_pgm" ;
    std::cout<<"export folder is \""<<pgmfolderpath<<"\""<<std::endl;
    if ( !boost::filesystem::exists( pgmfolderpath) ) {
        assert( boost::filesystem::create_directory( pgmfolderpath ) ) ;
    } else {
        assert( boost::filesystem::is_directory( pgmfolderpath) ) ;
    }
    for ( uint s = 0 ; s < data->n_slices; s++ ) {
        /// if corresponding segmentation file does not exist
        boost::filesystem::path p = pgmfolderpath ;
        p /= dico[ id ].substr( pgmfoldername.size());
        if ( !boost::filesystem::exists( p) )
            assert( boost::filesystem::create_directory( p ) ) ;
        p /= QString("slice-%1.pgm").arg( s, 0, 10).toStdString() ;
        // do it always as threshold may have changed
        //if ( !boost::filesystem::exists( p ) )
            export_binary_slice_as_pgm( data->slice(s), range, th, p.string() ) ;
    }
}

void MainWindow::openNewBillon( const QString &fileName, const std::string &serie )
{
    //helpmetounderstand() ;
    if ( fileName.isEmpty() ) return ;

    if ( _billon != 0 )	{
        delete _billon;
        _billon = 0;
    }
    _billon = DicomReader::read(fileName,serie);

    if ( _billon != 0 )
    {
        _currentSerie = 0 ;
        while ( serie.compare( _seriesUID[_currentSerie] ) != 0 )
            _currentSerie++ ;

        _mainPix = QImage(_billon->n_cols, _billon->n_rows,QImage::Format_ARGB32);
        //_segmPix = QImage(_billon->n_cols, _billon->n_rows,QImage::Format_ARGB32);
        _currentSlice = 0 ;
        _ui->sequenceSlider->setMaximum( 0 );
        _ui->sequenceSlider->setMaximum( _billon->n_slices-1 );
        _ui->sequenceSlider->setSliderPosition(_currentSlice);
        _ui->sequenceSlider->setEnabled(true);
        _ui->spinMinIntensity->setEnabled(true);
        _ui->spinMaxIntensity->setEnabled(true);
        _ui->binSpinBox->setValue( 128 );
        //export_segmentation( _seriesUID, _billon, _currentSerie,_ui->binSpinBox->value()) ;

    }
    else
    {
        _mainPix = QImage(0,0,QImage::Format_ARGB32);
        //_segmPix = QImage(0,0,QImage::Format_ARGB32);
        _currentSerie = -1 ;
    }

}


void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    openNewBillon( _currentRepository, item->text().toStdString() ) ;
    _ui->spinMinIntensity->setRange( _billon->minValue(), _billon->maxValue() );
    _ui->spinMaxIntensity->setRange( _billon->minValue(), _billon->maxValue() );

    _ui->spinMinIntensity->setValue( _billon->minValue() );
    _ui->spinMaxIntensity->setValue( _billon->maxValue() );
    drawSlice();
    _ui->segmLoadButton->setEnabled(true);
    _ui->skelLoadButton->setEnabled(true);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if ( _billon != 0 ) {
        if ( obj == _ui->_labelSliceView ) {
            if ( event->type() == QEvent::MouseMove) {
                    const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
                    QPoint curPixel = mouseEvent->pos() ;
                    curPixel /= _zoomFactor ;
                    if ( curPixel.x() < _billon->n_cols && curPixel.y() < _billon->n_rows )
                        on__labelSliceView_customContextMenuRequested( curPixel ) ;
            } else if ( event->type() == QEvent::Wheel ) {
                const QWheelEvent *wheelEvent = static_cast<const QWheelEvent*>(event);
                const int wheelDelta = wheelEvent->delta();
                if ( wheelDelta != 0 && (wheelEvent->modifiers() & Qt::ControlModifier) )
                {
                    _zoomFactor += wheelDelta>0 ? _zoomFactor*0.20 : -_zoomFactor*0.20;
                   // emit zoomFactorChanged(_zoomFactor,wheelEvent->globalPos());
                    drawSlice(false);
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_sequenceSlider_sliderMoved(int position)
{
    _currentSlice = position ;
    _ui->SlicePosition->setNum(position);
    drawSlice();
}

void MainWindow::on_sequenceSlider_valueChanged(int value)
{
    on_sequenceSlider_sliderMoved(value);
}


void MainWindow::on_binPushButton_clicked()
{
    export_segmentation( _seriesUID, _billon, _currentSerie, Interval<int>( _ui->spinMinIntensity->value(), _ui->spinMaxIntensity->value() ), _ui->binSpinBox->value()) ;
}

void MainWindow::on_spinMinIntensity_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_spinMaxIntensity_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    _bViewSegm = !_bViewSegm ;
    drawSlice();
}

void MainWindow::on_binSpinBox_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_segmLoadButton_clicked()
{
  if ( _billon == 0 ) return ;
  std::string pgmfoldername = set_pgmfoldername(_seriesUID);
  boost::filesystem::path pathImport = QDir::homePath().toStdString() ;
  pathImport /= "outputData";
  pathImport /= pgmfoldername  +"_pgm" ;
  pathImport /=_seriesUID[ _currentSerie ].substr( pgmfoldername.size()) ;
  std::cout<<"Trying opening folder "<<QString("%1").arg( pathImport.string().c_str() ).toStdString()<<std::endl;
  QString fileName = QFileDialog::getOpenFileName(0,tr("select pgm3d file"),QString("%1").arg( pathImport.string().c_str() ),tr("3D Image Files (*.pgm *.pgm3d)"));
  if ( !fileName.isEmpty() )
  {
      if ( _segmImg ) delete _segmImg ;
      _segmImg = factory.read( fileName ) ;
      if ( _segmImg != 0 ) {
          _ui->segmCheckBox->setEnabled(true);
          _ui->x_shift->setMaximum( _billon->n_rows - _segmImg->n_rows);
          _ui->y_shift->setMaximum( _billon->n_cols - _segmImg->n_cols);
          _ui->z_shift->setMaximum( _billon->n_slices - _segmImg->n_slices);
          _ui->x_shift->setValue(0);
          _ui->y_shift->setValue(0);
          _ui->z_shift->setValue(0);
          _ui->x_shift->setEnabled(true);
          _ui->y_shift->setEnabled(true);
          _ui->z_shift->setEnabled(true);
          _ui->contentCheckBox->setEnabled(true);
      }
  }
  if ( _ui->segmCheckBox->isChecked() ) drawSlice();

}

void MainWindow::on_segmCheckBox_stateChanged(int arg1)
{
    drawSlice();
    _ui->contentCheckBox->setEnabled( ( _ui->segmCheckBox->isChecked() ) ) ;
}

void MainWindow::on_x_shift_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_y_shift_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_z_shift_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_contentCheckBox_stateChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_skelLoadButton_clicked()
{
  if ( _billon == 0 ) return ;
  std::string pgmfoldername = set_pgmfoldername(_seriesUID);
  boost::filesystem::path pathImport = QDir::homePath().toStdString() ;
  pathImport /= "outputData";
  pathImport /= pgmfoldername  +"_pgm" ;
  pathImport /=_seriesUID[ _currentSerie ].substr( pgmfoldername.size()) ;
  std::cout<<"Trying opening folder "<<QString("%1").arg( pathImport.string().c_str() ).toStdString()<<std::endl;
  QString fileName = QFileDialog::getOpenFileName(0,tr("select pgm3d file"),QString("%1").arg( pathImport.string().c_str() ),tr("3D Image Files (*.pgm *.pgm3d)"));
  if ( !fileName.isEmpty() )
  {
      if ( _skelImg ) delete _skelImg ;
      _skelImg = factory.read( fileName ) ;
      if ( _skelImg != 0 ) {
          _ui->skelCheckBox->setEnabled(true);
          _ui->x_shift_skel->setMaximum( _billon->n_rows - _skelImg->n_rows);
          _ui->y_shift_skel->setMaximum( _billon->n_cols - _skelImg->n_cols);
          _ui->z_shift_skel->setMaximum( _billon->n_slices - _skelImg->n_slices);
          _ui->x_shift_skel->setValue(0);
          _ui->y_shift_skel->setValue(0);
          _ui->z_shift_skel->setValue(0);
          _ui->x_shift_skel->setEnabled(true);
          _ui->y_shift_skel->setEnabled(true);
          _ui->z_shift_skel->setEnabled(true);
          _ui->contentSkelCheckBox->setEnabled(true);
      }
  }
  if ( _ui->skelCheckBox->isChecked() ) drawSlice();


}

void MainWindow::on_y_shift_skel_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_z_shift_skel_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_x_shift_skel_valueChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_contentSkelCheckBox_stateChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on_skelCheckBox_stateChanged(int arg1)
{
    drawSlice();
}

void MainWindow::on__labelSliceView_customContextMenuRequested(const QPoint &pos)
{
    /// apport d'info contextuelle
    _ui->infoLabel->setText( QString("mouse on %1,%2,%3").arg(pos.x()).arg(pos.y()).arg(_currentSlice) );
}
