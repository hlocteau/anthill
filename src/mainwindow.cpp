#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "billon.h"
#include "coordinate.h"

#include "io/AntHillFile.hpp"
#include "geom2d.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QMouseEvent>
#include <RessourceDelegate.hpp>

#include "DGtal/images/ImageSelector.h"
#include "DGtal/io/writers/PNMWriter.h"
#include "DGtal/io/colormaps/GrayscaleColorMap.h"

namespace fs = boost::filesystem ;

bool USE_SEGM_TOOL = true ;

void MainWindow::onChangeBoolParameter(bool v) {
	drawSlice(v==v);
}
void MainWindow::onChangeIntParameter(int v) {
	drawSlice(v==v);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow) {
	
	_currentSlice = 0;
	_currentSerie = -1 ;
	_ui->setupUi(this);
	_bViewSegm = false ;
	_ui->_labelSliceView->installEventFilter(this);

	_ui->sequenceSlider->setEnabled(false);
	_ui->spinMinIntensity->setEnabled(false);
	_ui->spinMaxIntensity->setEnabled(false);

	QStringList horLabels ;
	horLabels << "Key" << "Value" ;
	_ui->tableWidget->setHorizontalHeaderLabels( horLabels ) ;
	_ui->axisSelection->addItem(tr("YZ")) ;
	_ui->axisSelection->addItem(tr("ZX")) ;
	_ui->axisSelection->addItem(tr("XY")) ;
	_ui->ressources->setMinimumWidth ( 150*3 ) ;
	initRessources(_ui->ressources) ;

	_segmImg = 0 ;
	_skelImg = 0 ;
	_zoomFactor = 1 ;

	connect( _ui->spinMinIntensity, SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->spinMaxIntensity, SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->binSpinBox,       SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->x_shift,          SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->y_shift,          SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->z_shift,          SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->x_shift_skel,     SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->y_shift_skel,     SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->z_shift_skel,     SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	
	connect( _ui->segmCheckBox,     SIGNAL(toggled(bool)), this, SLOT( onChangeBoolParameter(bool) ) ) ;
	connect( _ui->contentCheckBox,  SIGNAL(toggled(bool)), this, SLOT( onChangeBoolParameter(bool) ) ) ;
	connect( _ui->checkBox,         SIGNAL(toggled(bool)), this, SLOT( onChangeBoolParameter(bool) ) ) ;

	connect( _ui->axisSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeAxis(int)));

}

void MainWindow::onChangeAxis( int idx ) {
	if ( antHillMng.project() == 0 ) return ;
	uint dims[3] ;
	antHillMng.getSize( dims[1], dims[0], dims[2] ) ;
	
	_mainPix = QImage( dims[ (idx+1)%3], dims[ (idx+2)%3], QImage::Format_ARGB32);
	//std::cout<<"[ Info ] : image's dimensions are "<<dims[0]<<" x "<<dims[1]<<" x "<<dims[2]<<std::endl
	//         <<"           pixmap's size is "<<_mainPix.width()<<" x "<<_mainPix.height()<<std::endl
	//         <<"           slicing between 0 and "<<dims[idx]<<std::endl;
	_currentSlice = 0 ;
	_ui->sequenceSlider->setMaximum( 0 );
	_ui->sequenceSlider->setMaximum( dims[idx]-1 );
	_ui->sequenceSlider->setSliderPosition(_currentSlice);
	drawSlice( true ) ;
}

MainWindow::~MainWindow() {
	on_actionClose_folder_triggered() ;
	delete _ui;
}

void MainWindow::on_actionClose_folder_triggered() {

	_ui->sequenceSlider->setSliderPosition(0);
	_ui->spinMinIntensity->setValue(0);
	_ui->spinMaxIntensity->setValue(0);
	_ui->sequenceSlider->setEnabled(false);
	_ui->spinMinIntensity->setEnabled(false);
	_ui->spinMaxIntensity->setEnabled(false);

	antHillMng.reset() ;
	updateProjectsList() ;
	updateDictionary() ;
	resetRessources() ;
	closeImage();
}

void MainWindow::on_actionExit_triggered(){
   close();
}

void MainWindow::updateProjectsList() {
	_ui->listWidget->clear();
	for ( QVector< QString >::ConstIterator iterSerie = antHillMng.series_begin() ; iterSerie != antHillMng.series_end() ; iterSerie++ ) {
		_ui->listWidget->addItem( *iterSerie );
	}
}

void MainWindow::on_actionOpen_folder_triggered() {
	QString folderName = QFileDialog::getExistingDirectory(0,tr("select DICOM folder (import)"),QDir::homePath(),QFileDialog::ShowDirsOnly);
	if ( !folderName.isEmpty() ) {
		closeImage();
		antHillMng.importDicom( folderName ) ;
		updateProjectsList();
	}
}

void MainWindow::closeImage() {
	if ( _segmImg != 0 ) {
		delete _segmImg;
		_segmImg = 0 ;
	}
	if ( _skelImg != 0 ) {
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
	_mainPix = QImage(1,1,QImage::Format_ARGB32);
	drawSlice();
}

QColor MainWindow::getColorOf( const QString & search ) {
	uint row ;
	QTableWidgetItem *item = 0 ;
	for ( row = 0 ; row < (uint)_ressourcesTable->rowCount() ; row++ ) {
		item = _ressourcesTable->item( row, 0 ) ;
		if ( item->text() == search ) {
			item = _ressourcesTable->item( row, 2 ) ;
			return item->background().color() ;
		}
	}
	std::cerr<<"[ Warning ] : can not retrieve color for ressource "<<search.toStdString()<<std::endl;
	return QColor( 0,0,0 ) ;
}

QColor MainWindow::getColorOf( uint row ) {
	assert( row < (uint)_ressourcesTable->rowCount() ) ;
	QTableWidgetItem *item = _ressourcesTable->item( row, 2 ) ;
	return item->background().color() ;
}

void MainWindow::drawSlice( bool newContent ){

	if ( antHillMng.project() != 0 )	{
		if ( newContent ) {
			Interval<int> range_img (_ui->spinMinIntensity->value(),_ui->spinMaxIntensity->value());
			Interval<int> range_bin (0,255);
			_mainPix.fill(0xff000000);
			bool preview_binarization = _ui->checkBox->isChecked() ;
			bool is_first_layer = true ;
			for ( uint row = 0 ; row < _ressourcesTable->rowCount() ; row++ ) {
				QString resname = _ressourcesTable->item(row,0)->text() ;
				if ( ( _ressourcesTable->item(row,0)->checkState() != Qt::Checked && !preview_binarization ) ||
					( preview_binarization && resname != antHillMng.inputuid() ) ){
					//std::cout<<"[ info ] : do not draw ressource "<<resname.toStdString()<<" ( == "<< _ressourcesTable->item(row,0)->data( Qt::UserRole ).toString().toStdString() <<" )"<<std::endl;
					continue ;
				}
				arma::Mat<uint8_t> arma_mainPix( _mainPix.height(), _mainPix.width() ) ;
				arma_mainPix.fill(0);

				if ( !_ressourcesTable->item(row,2)->data( Qt::UserRole ).toBool() )
					antHillMng.draw( resname, arma_mainPix, _ui->axisSelection->currentIndex(), _currentSlice, range_img ) ;
				else
					antHillMng.draw( resname, arma_mainPix, _ui->axisSelection->currentIndex(), _currentSlice, range_bin ) ;
				resname = _ressourcesTable->item(row,0)->text() ;
				if ( _ui->axisSelection->currentIndex() != 0 )
					arma_mainPix = arma_mainPix.t() ;
				QRgb * writeIter = (QRgb *) _mainPix.bits() ;
				if ( !_ressourcesTable->item(row,2)->data( Qt::UserRole ).toBool() ) {
					if ( !preview_binarization ) {
						for ( arma::Mat<uint8_t>::iterator readIter = arma_mainPix.begin() ; readIter != arma_mainPix.end() ; readIter++ ) {
							if ( is_first_layer ) /// depending on whether we draw or we draw OVER
								* writeIter = qRgb( *readIter,*readIter,*readIter) ;
							else {
								if ( *readIter )
									*writeIter = qRgb( *readIter,*readIter,*readIter) ;
							}
							writeIter++ ;
						}
					} else {
						for ( arma::Mat<uint8_t>::iterator readIter = arma_mainPix.begin() ; readIter != arma_mainPix.end() ; readIter++ ) {
							if ( *readIter < _ui->binSpinBox->value() )
								* writeIter = qRgb( *readIter,*readIter,*readIter) ;
							else {
								/// alpha compositing with alpha=0.8 and color=QRgb(0,0,255)
								* writeIter = qRgb( 0.2 * *readIter, 0.2 * *readIter, 255*0.8 + 0.2 * *readIter ) ;
							}
							writeIter++ ;
						}
					}
				} else {
					for ( arma::Mat<uint8_t>::iterator readIter = arma_mainPix.begin() ; readIter != arma_mainPix.end() ; readIter++ ) {
						if ( is_first_layer ) /// depending on whether we draw or we draw OVER
							* writeIter = *readIter ? getColorOf( resname ).rgb() : qRgb(0,0,0) ;
						else {
							if ( *readIter )
								* writeIter = getColorOf( resname ).rgb() ;
						}
						writeIter++ ;
					}
				}
				is_first_layer = false ;
				if ( _ui->axisSelection->currentIndex() != 0 )
					arma_mainPix = arma_mainPix.t() ;
			}			
			/*
			if ( !_bViewSegm )
			   _sliceView->drawSlice(_mainPix,*_billon,_currentSlice, range_img, range_img.max());
			else
			   _sliceView->drawSlice(_mainPix,*_billon,_currentSlice, range_img, _ui->binSpinBox->value());


			if ( _segmImg && _ui->segmCheckBox->isChecked() ) {
				_sliceView->drawOverSlice(_mainPix,*_segmImg, _currentSlice, _ui->x_shift->value(), _ui->y_shift->value(),_ui->z_shift->value(), _ui->contentCheckBox->isChecked(), qRgb(255,255,0));
			}

			if ( _skelImg && _ui->skelCheckBox->isChecked() ) {
			 _sliceView->drawOverSlice(_mainPix,*_skelImg, _currentSlice, _ui->x_shift_skel->value(), _ui->y_shift_skel->value(),_ui->z_shift_skel->value(), _ui->contentSkelCheckBox->isChecked(),qRgb(0,0,255));
			}*/
		}
	}else{
		_ui->SlicePosition->setText(tr("Position"));
		_mainPix = QImage(1,1,QImage::Format_ARGB32);
		_mainPix.fill(0xff000000);
	}
	_ui->_labelSliceView->setPixmap( QPixmap::fromImage(_mainPix).scaled(_mainPix.width()*_zoomFactor,_mainPix.height()*_zoomFactor,Qt::KeepAspectRatio) );
}

void MainWindow::changeRessourcesConfigView(int row, int col) {
	QIcon icon;

	//for (int row = 0; row < _ressourcesTable->rowCount(); ++row) {
		QTableWidgetItem *item0 = _ressourcesTable->item(row, 0);
		if ( item0 == 0 ) return ;
		
		QTableWidgetItem *item1 = _ressourcesTable->item(row, 1);
		QTableWidgetItem *item2 = _ressourcesTable->item(row, 2);
		QTableWidgetItem *item3 = _ressourcesTable->item(row, 3);

		std::cout<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;	
		_ressourcesTable->cellWidget(row, 1)->setEnabled( item0->checkState() == Qt::Checked && !item1->data(Qt::UserRole).toBool() ) ;
		
		if (item0->checkState() == Qt::Checked) {
			QString fileName = item0->data(Qt::UserRole).toString();
			if ( ! findChild<QPushButton*>( "autoRefresh" )->isChecked() )
				std::cout<<"image "<<fileName.toStdString()<<" has to be drawn "<<item1->text().toStdString()<<(item3->checkState() == Qt::Checked?" and ":" but not ")<<"tracked / color "<<item2->background().color().red()<<":"<<item2->background().color().green()<<":"<<item2->background().color().blue()<<std::endl;
		}
	//}
	//previewArea->setIcon(icon);
	std::cout<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
	if ( !findChild<QPushButton*>( "autoRefresh")->isChecked() )
		drawSlice(true);
}

void MainWindow::toggled_config_view(bool checked ) {
	QPushButton* autoRefresh = findChild<QPushButton*>( "autoRefresh") ;
	if ( !checked ) {
		autoRefresh->setText( "Refresh" ) ;
	} else {
		autoRefresh->setText( "Freeze" ) ;
		drawSlice(true);
	}
}

void MainWindow::initRessources( QWidget *parent ) {
	_ressourcesGroupBox = new QGroupBox(tr("Available ressources"),parent);

	_ressourcesTable = new QTableWidget;
	_ressourcesTable->setMinimumWidth( parent->minimumWidth()*0.9 ) ;
	_ressourcesTable->setSelectionMode(QAbstractItemView::NoSelection);
	_ressourcesTable->setItemDelegate(new RessourceDelegate(this));

	QStringList labels;
	labels << tr("Image") << tr("Mode") << tr("Color") << tr("Tracked");

	_ressourcesTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
	_ressourcesTable->setColumnCount(4);
	_ressourcesTable->setHorizontalHeaderLabels(labels);
	_ressourcesTable->verticalHeader()->hide();

	connect(_ressourcesTable, SIGNAL(cellChanged(int,int)), this, SLOT(changeRessourcesConfigView(int,int)));
	connect( _ressourcesTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(changeRessourceColor(int,int)) ) ;
	QVBoxLayout *layout = new QVBoxLayout;
	
	QPushButton *autoRefresh = new QPushButton(tr("Refresh"));
	autoRefresh->setObjectName( "autoRefresh") ;
	autoRefresh->setCheckable(true);
	//autoRefresh->setChecked ( true ) ;
	connect( autoRefresh, SIGNAL(toggled(bool)), this, SLOT(toggled_config_view(bool)));
	layout->addWidget(autoRefresh);
	layout->addWidget(_ressourcesTable);
	_ressourcesGroupBox->setLayout(layout);
}

void MainWindow::changeRessourceColor(int row,int column) {
	if ( column != 2 ) return ;
	if ( _ressourcesTable->item( row, 0 )->checkState() != Qt::Checked ) return ;
	if ( ! _ressourcesTable->item( row, 2 )->data( Qt::UserRole).toBool() ) return ;
	QColor new_color = QColorDialog::getColor( _ressourcesTable->item( row, column )->background().color(),
												this, tr("Pick a new color"), QColorDialog::DontUseNativeDialog ) ;
	if ( new_color.isValid() && new_color != Qt::black ) {
		_ressourcesTable->item( row, column )->setBackground( QBrush(new_color) ) ;
		drawSlice( true ) ;
	}
}

const double golden_ratio_conjugate = 0.618033988749895 ;
QColor makeRgbColor( double &hue ) {
  hue += golden_ratio_conjugate ;
  if ( hue > 1. ) hue -= 1. ;
  return  QColor::fromHsv( (int)floor(hue*359), 76, 252) ;
}

void MainWindow::updateRessources( ) {
	std::cout<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
	if ( antHillMng.project() ) {
		_ressourcesTable->setRowCount(0) ;
		QMap< QString, QMap< QString,QString > >::ConstIterator resultIter = antHillMng.project()->process_begin(),
																resultEnd = antHillMng.project()->process_end();
		QMap< QString,QString >::ConstIterator fieldIter, fieldEnd ;
		double hue=( rand() % 1000 ) / 1000. ;
		for ( ; resultIter != resultEnd ; resultIter++ ) {
			fieldIter = resultIter.value().begin();
			fieldEnd = resultIter.value().end();
			for ( ; fieldIter != fieldEnd ; fieldIter ++ ) {
				if ( !fieldIter.key().startsWith("result") ) continue ;
std::cout<<"[ Info ] : process ("<<resultIter.key().toStdString()<<" ( "<<fieldIter.key().toStdString()<<" : "<<fieldIter.value().toStdString()<<" ) )"<<std::endl;
				int row = _ressourcesTable->rowCount();
				_ressourcesTable->setRowCount(row + 1);

				QTableWidgetItem *item0 = new QTableWidgetItem( antHillMng.uid( resultIter, fieldIter ) );
				item0->setData(Qt::UserRole, fieldIter.value().split(";").at(0) );
				item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
				QTableWidgetItem *item1 = new QTableWidgetItem(tr("Content"));
				/**
				 * \brief it makes no sense to define a boundary on feature images
				 */
				item1->setData( Qt::UserRole, antHillMng.isContentOnly( fieldIter ) ) ;
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
				QTableWidgetItem *item2 = new QTableWidgetItem();
				/**
				 * \brief we only define new color(s) for bilevel/labelled images
				 */
				item2->setData( Qt::UserRole, antHillMng.isColorSelectionAllowed( fieldIter ) ) ;
				if ( antHillMng.isColorSelectionAllowed( fieldIter ) ) {
					item2->setBackground ( QBrush( makeRgbColor( hue ) ) ) ;
std::cout<<"[ Debug ] : set color for "<<fieldIter.key().toStdString()<<" : "<<fieldIter.value().toStdString()<<std::endl;
				}
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
				QTableWidgetItem *item3 = new QTableWidgetItem();
				item3->setCheckState(Qt::Unchecked);
				item3->setTextAlignment( Qt::AlignCenter ) ;

				/** \warning the item0 is the last one to be inserted in the current row
				 *           wrt the slot devoted to signal cellChanged
				 */
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
				_ressourcesTable->setItem(row, 1, item1);
				_ressourcesTable->openPersistentEditor(item1);
				_ressourcesTable->setItem(row, 2, item2);
				_ressourcesTable->setItem(row, 3, item3);
				item0->setCheckState(Qt::Unchecked);
std::cout<<"           will trigger slot..."<<std::endl;
				_ressourcesTable->setItem(row, 0, item0);
			}
		}
		_ressourcesTable->resizeRowsToContents () ;
	}
	std::cout<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
}

void MainWindow::resetRessources() {
	_ressourcesTable->setRowCount(0) ;
	//changeRessourcesConfigView() ;
}
 
void MainWindow::updateDictionary( ) {
	if ( antHillMng.project() ) {
		const QMap< QString, QString > & dictionary = antHillMng.project()->dictionary() ;
		QMap< QString,QString>::ConstIterator keyValue ;
		uint iRow = 0 ;
		if ( _ui->tableWidget->rowCount() != dictionary.size() )
			_ui->tableWidget->setRowCount( dictionary.size() ) ;
		for ( keyValue = dictionary.begin() ; keyValue != dictionary.end() ; keyValue++,iRow++ ) {
			if ( !_ui->tableWidget->item( iRow, 0) ) {
				_ui->tableWidget->setItem(iRow,0,new QTableWidgetItem(""));
				_ui->tableWidget->setItem(iRow,1,new QTableWidgetItem(""));
			}
			_ui->tableWidget->item( iRow, 0)->setText( keyValue.key() ) ;
			_ui->tableWidget->item( iRow, 1)->setText( keyValue.value() ) ;
		}
	} else {
		_ui->tableWidget->setRowCount( 0 ) ;
	}
}

void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item) {
	if ( !antHillMng.load( item->text() ) ) {
		_mainPix = QImage(1,1,QImage::Format_ARGB32);
		_currentSerie = -1 ;
		return ;
	}
	updateDictionary( );
	updateRessources( ) ;
	uint idx = _ui->axisSelection->currentIndex();
	uint dims[3] ;
	antHillMng.getSize( dims[1], dims[0],dims[2] ) ;
	_mainPix = QImage( dims[ (idx+1)%3], dims[ (idx+2)%3], QImage::Format_ARGB32);
	_ui->sequenceSlider->setMaximum( 0 );
	_ui->sequenceSlider->setMaximum( dims[idx]-1 );
	_ui->sequenceSlider->setSliderPosition(_currentSlice);
	
	_ui->sequenceSlider->setEnabled(true);
	_ui->spinMinIntensity->setEnabled(true);
	_ui->spinMaxIntensity->setEnabled(true);

	Interval<arma::s16> range ;
	antHillMng.getRange<arma::s16>( range ) ;//range.setBounds(0,5161);
	_ui->spinMinIntensity->setRange( range.min(), range.max() );
	_ui->spinMaxIntensity->setRange( range.min(), range.max() );
	_ui->spinMinIntensity->setValue( range.min() );
	_ui->spinMaxIntensity->setValue( range.max() );
	
	_ui->binSpinBox->setValue( 128 );
	
	drawSlice();
	_ui->segmLoadButton->setEnabled(true);
	_ui->skelLoadButton->setEnabled(true);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if ( antHillMng.project() != 0 ) {
		if ( obj == _ui->_labelSliceView ) {
			if ( event->type() == QEvent::MouseMove) {
				   const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
				   QPoint curPixel = mouseEvent->pos() ;
				   curPixel /= _zoomFactor ;
				   if ( curPixel.x() < _mainPix.width() && curPixel.y() < _mainPix.height() )
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
			} else if ( event->type() == QEvent::MouseButtonPress ) {
				const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
				if ( mouseEvent->button() == Qt::RightButton ) {
					QPoint curPixel = mouseEvent->pos() ;
					curPixel /= _zoomFactor ;
					if ( curPixel.x() < _mainPix.width() && curPixel.y() < _mainPix.height() ) {
						/// cyclic rotation of axis selection
						_ui->axisSelection->setCurrentIndex( (_ui->axisSelection->currentIndex() + 1 ) % 3 ) ;
						std::cout<<"[ Info ] : set slice position (turn around voxel ["<<_ui->infoLabel->text().toStdString()<<"])"<<std::endl;
						on_sequenceSlider_sliderMoved( curPixel.x() );
					}
				}
			} else if ( event->type() == QEvent::KeyPress ) {
				const QKeyEvent *keyEvent = static_cast< const QKeyEvent*>( event ) ;
				const int key = keyEvent->key() ;
				if ( ( key == Qt::Key_Plus || key == Qt::Key_Minus ) && ( keyEvent->modifiers() & Qt::ControlModifier ) ) {
					_zoomFactor += key == Qt::Key_Plus ? _zoomFactor*0.20 : -_zoomFactor*0.20;
					drawSlice(false);
				}
			}
		}
	}
	return QMainWindow::eventFilter(obj, event);
}

void MainWindow::on_sequenceSlider_sliderMoved(int position) {
	_currentSlice = position ;
	if ( position != _ui->sequenceSlider->value() )
		_ui->sequenceSlider->setValue( position ) ;
	_ui->SlicePosition->setNum(position);
	drawSlice();
}

void MainWindow::on_sequenceSlider_valueChanged(int value) {
	on_sequenceSlider_sliderMoved(value);
}

void MainWindow::on_binPushButton_clicked() {
	antHillMng.binarization( Interval<arma::s16>(_ui->spinMinIntensity->value(), _ui->spinMaxIntensity->value() ), _ui->binSpinBox->value()) ;
	updateRessources() ;
}

void MainWindow::on_checkBox_stateChanged(int arg1){
	_bViewSegm = !_bViewSegm ;
	drawSlice();
}
void MainWindow::on_segmLoadButton_clicked(){
	if ( antHillMng.project() == 0 ) return ;

	QString fileName = QFileDialog::getOpenFileName(0,tr("select pgm3d file"),QString("%1").arg( antHillMng.projectLocation().c_str() ),tr("3D Image Files (*.pgm *.pgm3d)"));
	if ( !fileName.isEmpty() ) {
		if ( _segmImg ) delete _segmImg ;
		_segmImg = factory.read( fileName ) ;
		if ( _segmImg != 0 ) {
			uint dims[3] ;
			antHillMng.getSize( dims[1], dims[0], dims[2] ) ;
			_ui->segmCheckBox->setEnabled(true);
			_ui->x_shift->setMaximum( dims[0] - _segmImg->n_rows);
			_ui->y_shift->setMaximum( dims[1] - _segmImg->n_cols);
			_ui->z_shift->setMaximum( dims[2] - _segmImg->n_slices);
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

void MainWindow::on_segmCheckBox_stateChanged(int arg1){
	drawSlice();
	_ui->contentCheckBox->setEnabled( ( _ui->segmCheckBox->isChecked() ) ) ;
}

void MainWindow::on_skelLoadButton_clicked(){
	if ( antHillMng.project() == 0 ) return ;
	QString fileName = QFileDialog::getOpenFileName(0,tr("select pgm3d file"),QString("%1").arg( antHillMng.projectLocation().c_str() ),tr("3D Image Files (*.pgm *.pgm3d)"));
	if ( !fileName.isEmpty() ) {
		if ( _skelImg ) delete _skelImg ;
		_skelImg = factory.read( fileName ) ;
		if ( _skelImg != 0 ) {
			uint dims[3] ;
			antHillMng.getSize( dims[1], dims[0], dims[2] ) ;
			_ui->skelCheckBox->setEnabled(true);
			_ui->x_shift_skel->setMaximum( dims[0] - _skelImg->n_rows);
			_ui->y_shift_skel->setMaximum( dims[1] - _skelImg->n_cols);
			_ui->z_shift_skel->setMaximum( dims[2] - _skelImg->n_slices);
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
void MainWindow::on__labelSliceView_customContextMenuRequested(const QPoint &pos){
	/// apport d'info contextuelle
	uint16_t location[3] ;
	uint8_t idx = _ui->axisSelection->currentIndex() ;
	location[ (idx-1+3)%3 ] = pos.y() ;
	location[ (idx-2+3)%3 ] = pos.x() ;

	location[ idx ] = _currentSlice ;
	_ui->infoLabel->setText( QString("mouse on %1,%2,%3").arg(location[0]).arg(location[1]).arg(location[2]) );
}

void MainWindow::on_actionOpen_project_triggered() {
	QString fileName = QFileDialog::getOpenFileName(0,tr("select project file"), QString("%1").arg( antHillMng.defaultProjectLocation().c_str() ),tr("Anthill project Files (*.xml)"));
	if ( !fileName.isEmpty() ) {
		on_actionClose_folder_triggered();
		antHillMng.setFileName( fileName ) ;
		updateProjectsList();
		// as we get only one serie...
		on_listWidget_itemDoubleClicked( _ui->listWidget->item(0) ) ;
	}
}
