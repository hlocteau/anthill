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

	_ui->axisSelection->addItem(tr("XY")) ;
	_ui->axisSelection->addItem(tr("YZ")) ;
	_ui->axisSelection->addItem(tr("XZ")) ;

	_ui->ressources->setMinimumWidth ( 150*3 ) ;
	initRessources(_ui->ressources) ;

	_billon = 0 ;
	_segmImg = 0 ;
	_skelImg = 0 ;
	_zoomFactor = 1 ;
}

MainWindow::~MainWindow() {
	on_actionClose_folder_triggered() ;
	delete _ui;
}

void MainWindow::on_actionClose_folder_triggered() {
	_currentRepository.clear() ;

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
	if ( _billon != 0 ) {
		delete _billon;
		_billon = 0;
	}
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
	_mainPix = QImage(0,0,QImage::Format_ARGB32);
	drawSlice();
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

void MainWindow::changeRessourcesConfigView(int row, int col) {
	QIcon icon;

	//for (int row = 0; row < _ressourcesTable->rowCount(); ++row) {
		QTableWidgetItem *item0 = _ressourcesTable->item(row, 0);
		if ( item0 == 0 ) return ;
		
		QTableWidgetItem *item1 = _ressourcesTable->item(row, 1);
		QTableWidgetItem *item2 = _ressourcesTable->item(row, 2);
		QTableWidgetItem *item3 = _ressourcesTable->item(row, 3);

		std::cout<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;	
		_ressourcesTable->cellWidget(row, 1)->setEnabled( item0->checkState() == Qt::Checked ) ;
		
		if (item0->checkState() == Qt::Checked) {
			QIcon::Mode mode;
			if (item1->text() == tr("Normal")) {
				mode = QIcon::Normal;
			} else if (item1->text() == tr("Active")) {
				mode = QIcon::Active;
			} else if (item1->text() == tr("Disabled")) {
				mode = QIcon::Disabled;
			} else {
				mode = QIcon::Selected;
			}

			QString fileName = item0->data(Qt::UserRole).toString();
			if ( ! findChild<QPushButton*>( "autoRefresh" )->isChecked() )
				std::cout<<"image "<<fileName.toStdString()<<" has to be drawn "<<item1->text().toStdString()<<(item3->checkState() == Qt::Checked?" and ":" but not ")<<"tracked / color "<<item2->background().color().red()<<":"<<item2->background().color().green()<<":"<<item2->background().color().blue()<<std::endl;
		}
	//}
	//previewArea->setIcon(icon);
	std::cout<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
}

void MainWindow::toggled_config_view(bool checked ) {
	QPushButton* autoRefresh = findChild<QPushButton*>( "autoRefresh") ;
	if ( checked ) {
		autoRefresh->setText( "Freeze" ) ;
	} else {
		autoRefresh->setText( "Refresh" ) ;
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
/*	_ressourcesTable->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
	_ressourcesTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
	_ressourcesTable->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
	_ressourcesTable->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);*/
	_ressourcesTable->verticalHeader()->hide();

	connect(_ressourcesTable, SIGNAL(cellChanged(int,int)), this, SLOT(changeRessourcesConfigView(int,int)));
	connect( _ressourcesTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(changeRessourceColor(int,int)) ) ;
	QVBoxLayout *layout = new QVBoxLayout;
	
	QPushButton *autoRefresh = new QPushButton(tr("Freeze"));
	autoRefresh->setObjectName( "autoRefresh") ;
	autoRefresh->setCheckable(true);
	autoRefresh->setChecked ( true ) ;
	connect( autoRefresh, SIGNAL(toggled(bool)), this, SLOT(toggled_config_view(bool)));
	layout->addWidget(autoRefresh);
	layout->addWidget(_ressourcesTable);
	_ressourcesGroupBox->setLayout(layout);
}

void MainWindow::changeRessourceColor(int row,int column) {
	if ( column != 2 ) return ;
	if ( _ressourcesTable->item( row, 0 )->checkState() != Qt::Checked ) return ;
	QColor new_color = QColorDialog::getColor( _ressourcesTable->item( row, column )->background().color(),
												this, tr("Pick a new color"), QColorDialog::DontUseNativeDialog ) ;
	if ( new_color.isValid() && new_color != Qt::black )
		_ressourcesTable->item( row, column )->setBackground( QBrush(new_color) ) ;
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
		QMap< QString, QMap< QString,QString > >::const_iterator resultIter = antHillMng.project()->process_begin(),
																	resultEnd = antHillMng.project()->process_end();
		double hue=( rand() % 1000 ) / 1000. ;
		for ( ; resultIter != resultEnd ; resultIter++ ) {
			int row = _ressourcesTable->rowCount();
			_ressourcesTable->setRowCount(row + 1);

			QTableWidgetItem *item0 = new QTableWidgetItem( resultIter.key() );
			item0->setData(Qt::UserRole, resultIter.value().value( "result" ) );
			item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);

			QTableWidgetItem *item1 = new QTableWidgetItem(tr("Content"));
			QTableWidgetItem *item2 = new QTableWidgetItem();
			item2->setBackground ( QBrush( makeRgbColor( hue ) ) ) ;
			
			QTableWidgetItem *item3 = new QTableWidgetItem();
			item3->setCheckState(Qt::Unchecked);
			item3->setTextAlignment( Qt::AlignCenter ) ;

			/** \warning the item0 is the last one to be inserted in the current row
			 *           wrt the slot devoted to signal cellChanged
			 **/
			_ressourcesTable->setItem(row, 1, item1);
			_ressourcesTable->openPersistentEditor(item1);
			_ressourcesTable->setItem(row, 2, item2);
			_ressourcesTable->setItem(row, 3, item3);
			item0->setCheckState(Qt::Unchecked);
			_ressourcesTable->setItem(row, 0, item0);
			
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
		if ( _billon ) delete _billon ;
		_billon = 0 ;
	} else   
		antHillMng.openInitialInput( &_billon ) ;
	
	if ( !_billon ) {
		_mainPix = QImage(0,0,QImage::Format_ARGB32);
		_currentSerie = -1 ;
		 return ;
	}
	updateDictionary( );
	updateRessources( ) ;
	_mainPix = QImage(_billon->n_cols, _billon->n_rows,QImage::Format_ARGB32);
	_currentSlice = 0 ;
	_ui->sequenceSlider->setMaximum( 0 );
	_ui->sequenceSlider->setMaximum( _billon->n_slices-1 );
	_ui->sequenceSlider->setSliderPosition(_currentSlice);
	_ui->sequenceSlider->setEnabled(true);
	_ui->spinMinIntensity->setEnabled(true);
	_ui->spinMaxIntensity->setEnabled(true);

	_ui->spinMinIntensity->setRange( _billon->minValue(), _billon->maxValue() );
	_ui->spinMaxIntensity->setRange( _billon->minValue(), _billon->maxValue() );
	_ui->spinMinIntensity->setValue( _billon->minValue() );
	_ui->spinMaxIntensity->setValue( _billon->maxValue() );
	_ui->binSpinBox->setValue( 128 );
	
	drawSlice();
	_ui->segmLoadButton->setEnabled(true);
	_ui->skelLoadButton->setEnabled(true);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
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
	_ui->SlicePosition->setNum(position);
	drawSlice();
}

void MainWindow::on_sequenceSlider_valueChanged(int value) {
	on_sequenceSlider_sliderMoved(value);
}

void MainWindow::on_binPushButton_clicked() {
	antHillMng.binarization( _billon, Interval<__billon_type__>(_ui->spinMinIntensity->value(), _ui->spinMaxIntensity->value() ), _ui->binSpinBox->value()) ;
	updateRessources() ;
}

void MainWindow::on_spinMinIntensity_valueChanged(int arg1){
	drawSlice();
}

void MainWindow::on_spinMaxIntensity_valueChanged(int arg1){
	drawSlice();
}

void MainWindow::on_checkBox_stateChanged(int arg1){
	_bViewSegm = !_bViewSegm ;
	drawSlice();
}

void MainWindow::on_binSpinBox_valueChanged(int arg1){
	drawSlice();
}

void MainWindow::on_segmLoadButton_clicked(){
  if ( _billon == 0 ) return ;

  QString fileName = QFileDialog::getOpenFileName(0,tr("select pgm3d file"),QString("%1").arg( antHillMng.projectLocation().c_str() ),tr("3D Image Files (*.pgm *.pgm3d)"));
  if ( !fileName.isEmpty() ) {
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

void MainWindow::on_segmCheckBox_stateChanged(int arg1){
	drawSlice();
	_ui->contentCheckBox->setEnabled( ( _ui->segmCheckBox->isChecked() ) ) ;
}

void MainWindow::on_x_shift_valueChanged(int arg1){
	drawSlice();
}
void MainWindow::on_y_shift_valueChanged(int arg1){

	drawSlice();
}
void MainWindow::on_z_shift_valueChanged(int arg1){

	drawSlice();
}
void MainWindow::on_contentCheckBox_stateChanged(int arg1){

	drawSlice();
}
void MainWindow::on_skelLoadButton_clicked(){
	if ( _billon == 0 ) return ;
	QString fileName = QFileDialog::getOpenFileName(0,tr("select pgm3d file"),QString("%1").arg( antHillMng.projectLocation().c_str() ),tr("3D Image Files (*.pgm *.pgm3d)"));
	if ( !fileName.isEmpty() ) {
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
void MainWindow::on_y_shift_skel_valueChanged(int arg1){
	drawSlice();
}
void MainWindow::on_z_shift_skel_valueChanged(int arg1){
	drawSlice();
}
void MainWindow::on_x_shift_skel_valueChanged(int arg1){
	drawSlice();
}
void MainWindow::on_contentSkelCheckBox_stateChanged(int arg1){
	drawSlice();
}
void MainWindow::on_skelCheckBox_stateChanged(int arg1){
	drawSlice();
}
void MainWindow::on__labelSliceView_customContextMenuRequested(const QPoint &pos){
	/// apport d'info contextuelle
	_ui->infoLabel->setText( QString("mouse on %1,%2,%3").arg(pos.x()).arg(pos.y()).arg(_currentSlice) );
}

void MainWindow::on_actionOpen_project_triggered() {
	QString fileName = QFileDialog::getOpenFileName(0,tr("select project file"), QString("%1").arg( antHillMng.defaultProjectLocation().c_str() ),tr("Anthill project Files (*.xml)"));
	if ( !fileName.isEmpty() ) {
		closeImage();
		antHillMng.setFileName( fileName ) ;
		updateProjectsList();
		// as we get only one serie...
		on_listWidget_itemDoubleClicked( _ui->listWidget->item(0) ) ;
	}
}
