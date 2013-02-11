#include <mainwindow.h>
#include <ui_mainwindow.h>

#include <billon.h>
#include <def_coordinate.h>
#include <coordinate.h>

#include <io/AntHillFile.hpp>

#include <QFileDialog>
#include <QColorDialog>
#include <QMouseEvent>
#include <RessourceDelegate.hpp>
#include <QLineEdit>
#include <DGtal/images/ImageSelector.h>
#include <DGtal/io/colormaps/GrayscaleColorMap.h>

namespace fs = boost::filesystem ;

bool USE_SEGM_TOOL = true ;

QColor getLabelColor( arma::u32 iColor, const Interval<arma::u32> & selIdx ) ;

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
    /*
    _ui->spinMinIntensity->setEnabled(false);
	_ui->spinMaxIntensity->setEnabled(false);
    */
	QStringList horLabels ;
	horLabels << "Key" << "Value" ;
	_ui->tableWidget->setHorizontalHeaderLabels( horLabels ) ;
	_ui->axisSelection->addItem(tr("YZ")) ;
	_ui->axisSelection->addItem(tr("ZX")) ;
	_ui->axisSelection->addItem(tr("XY")) ;
	_ui->ressources->setMinimumWidth ( 150*3 ) ;
	initRessources(_ui->ressources) ;

    /*
    _segmImg = 0 ;
    _skelImg = 0 ;
    */
	_zoomFactor = 1 ;

    /*
    connect( _ui->spinMinIntensity, SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->spinMaxIntensity, SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
    */
    connect( _ui->binSpinBox,       SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
    /*
    connect( _ui->x_shift,          SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->y_shift,          SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->z_shift,          SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->x_shift_skel,     SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->y_shift_skel,     SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	connect( _ui->z_shift_skel,     SIGNAL(valueChanged(int)), this, SLOT( onChangeIntParameter(int) ) ) ;
	
	connect( _ui->segmCheckBox,     SIGNAL(toggled(bool)), this, SLOT( onChangeBoolParameter(bool) ) ) ;
	connect( _ui->contentCheckBox,  SIGNAL(toggled(bool)), this, SLOT( onChangeBoolParameter(bool) ) ) ;
    */
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
    _ui->sequenceSlider->setEnabled(false);
    /*
	_ui->spinMinIntensity->setValue(0);
	_ui->spinMaxIntensity->setValue(0);
	_ui->spinMinIntensity->setEnabled(false);
	_ui->spinMaxIntensity->setEnabled(false);
    */
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
    /*
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
    */
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

QColor getLabelColor( arma::u32 iColor, const Interval<arma::u32> & selIdx ) {
	arma::s32 stepColor = (int)floor( log( (double)selIdx.size()+1 ) / log( 3. ) + 1 );
	return QColor((255/stepColor)*(iColor/(stepColor*stepColor)), (255/stepColor)*( (iColor/stepColor) % stepColor ),( 255 / stepColor ) * ( iColor % stepColor) ) ;
}

template <typename T > Interval<T> RANGE( const QString &text ) {
	QStringList range_text = text.split(" ", QString::SkipEmptyParts) ;
	Interval<T> r;
	r.setBounds( range_text.front().split(":").front().toInt(), range_text.back().split(":").back().toInt() ) ;
	return r ;
}
void MainWindow::drawSlice( bool newContent ){

	if ( antHillMng.project() != 0 )	{
		if ( newContent ) {
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
                Interval<arma::s32> range_res( RANGE<arma::s32>( qobject_cast<QLineEdit *>(_ressourcesTable->cellWidget(row,3) )->text() ) ) ;
                arma::Mat<arma::u32> arma_mainPix ( _mainPix.height(), _mainPix.width() ) ;
				arma_mainPix.fill(0);

                if ( resname == antHillMng.inputuid() )
                    antHillMng.draw( resname, arma_mainPix, _ui->axisSelection->currentIndex(), _currentSlice, range_res ) ;
                else {
					QTableWidgetItem * item = _ressourcesTable->item( row,3 ) ;
					std::cout<<"Tooltip is ["<<item->toolTip().toStdString()<<"]"<<std::endl;
					antHillMng.draw( resname, arma_mainPix, _ui->axisSelection->currentIndex(), _currentSlice, RANGE<arma::s32>( item->toolTip() ), false ) ;
				}
				resname = _ressourcesTable->item(row,0)->text() ;
				if ( _ui->axisSelection->currentIndex() != 0 )
					arma_mainPix = arma_mainPix.t() ;
				QRgb * writeIter = (QRgb *) _mainPix.bits() ;
				if ( !_ressourcesTable->item(row,2)->data( Qt::UserRole ).toBool() ) {
					if ( !preview_binarization ) {
						for ( arma::Mat<arma::u32>::iterator readIter = arma_mainPix.begin() ; readIter != arma_mainPix.end() ; readIter++ ) {
							if ( is_first_layer ) /// depending on whether we draw or we draw OVER
								* writeIter = qRgb( *readIter,*readIter,*readIter) ;
							else {
								if ( *readIter )
									*writeIter = qRgb( *readIter,*readIter,*readIter) ;
							}
							writeIter++ ;
						}
					} else {
						for ( arma::Mat<arma::u32>::iterator readIter = arma_mainPix.begin() ; readIter != arma_mainPix.end() ; readIter++ ) {
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
					QMap< arma::u32, arma::u32 > counter ;
					for ( arma::Mat<arma::u32>::iterator readIter = arma_mainPix.begin() ; readIter != arma_mainPix.end() ; readIter++ ) {
						if ( range_res.containsClosed( *readIter ) ) {
							if ( !counter.contains(*readIter) ) counter.insert( *readIter, 0 ) ;
							counter[ *readIter ] ++ ;
							QColor cl = *readIter ? getLabelColor( *readIter, Interval<arma::u32>(range_res.min(),range_res.max() ) ) : qRgb(0,0,0) ;
							if ( is_first_layer ) /// depending on whether we draw or we draw OVER
								* writeIter = cl.rgb() ;
							else {
								if ( *readIter )
									* writeIter = cl.rgb() ;
							}
						}
						writeIter++ ;
					}
					for ( QMap< arma::u32, arma::u32 >::ConstIterator it = counter.begin() ; it != counter.end(); it++ )
						std::cout<<(int)it.key()<<" : "<<(int)it.value() <<std::endl;
				}
				is_first_layer = false ;
				std::cout<<"[ info ] : draw ressource "<<resname.toStdString()<<" ( = "<< range_res.min()<<":"<<range_res.max() <<" ) " <<_ressourcesTable->item(row,2)->data( Qt::UserRole ).toBool()<<std::endl;
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
    labels << tr("Image") << tr("Mode") << tr("Color") << tr("Selection")<<tr("Tracked");

	_ressourcesTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    _ressourcesTable->setColumnCount(labels.size());
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

void MainWindow::changeRessourceSelection() {
    QLineEdit * ql = qobject_cast<QLineEdit *>( sender() ) ;
    QStringList sl = ql->objectName().split("_") ;
    changeRessourcesConfigView( sl.back().toInt(), 3);
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

                QTableWidgetItem *itemUID = new QTableWidgetItem( antHillMng.uid( resultIter, fieldIter ) );
                itemUID->setData(Qt::UserRole, fieldIter.value().split(";").at(0) );
                itemUID->setFlags(itemUID->flags() & ~Qt::ItemIsEditable);
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
                QTableWidgetItem *itemMode = new QTableWidgetItem(tr("Content"));
				/**
				 * \brief it makes no sense to define a boundary on feature images
				 */
                itemMode->setData( Qt::UserRole, antHillMng.isContentOnly( fieldIter ) ) ;
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
                QTableWidgetItem *itemColor = new QTableWidgetItem();
				/**
				 * \brief we only define new color(s) for bilevel/labelled images
				 */
                itemColor->setData( Qt::UserRole, antHillMng.isColorSelectionAllowed( fieldIter ) ) ;
				if ( antHillMng.isColorSelectionAllowed( fieldIter ) ) {
                    itemColor->setBackground ( QBrush( makeRgbColor( hue ) ) ) ;
std::cout<<"[ Debug ] : set color for "<<fieldIter.key().toStdString()<<" : "<<fieldIter.value().toStdString()<<std::endl;
				}
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;

                QTableWidgetItem *itemFilterValues = new QTableWidgetItem( );
                itemFilterValues->setFlags(itemFilterValues->flags() | Qt::ItemIsEditable);
                itemFilterValues->setTextAlignment( Qt::AlignCenter ) ;
                Interval< arma::s32 > current_range ;
                antHillMng.getRange<arma::s32 >( itemUID->text(), current_range ) ;
                QTableWidgetItem *itemTrack = new QTableWidgetItem();
                itemTrack->setCheckState(Qt::Unchecked);
                itemTrack->setTextAlignment( Qt::AlignCenter ) ;

                /** \warning the item itemUID is the last one to be inserted in the current row
				 *           wrt the slot devoted to signal cellChanged
				 */
std::cout<<"[ Debug ] : "<<__FUNCTION__<<" @ line "<<__LINE__<<std::endl;
                _ressourcesTable->setItem(row, 1, itemMode);
                _ressourcesTable->openPersistentEditor(itemMode);
                _ressourcesTable->setItem(row, 2, itemColor);
                _ressourcesTable->setItem(row, 3, itemFilterValues);
                QLineEdit * rangeLine = new QLineEdit( QString("%1:%2").arg( current_range.min() ).arg( current_range.max() ) ) ;
                rangeLine->setObjectName ( QString ("cell_%1").arg(row) ) ;
                _ressourcesTable->setCellWidget(row, 3, rangeLine ) ;
                itemFilterValues->setToolTip( rangeLine->text()) ;
                connect( rangeLine, SIGNAL(returnPressed()), this, SLOT(changeRessourceSelection())) ;
                _ressourcesTable->setItem(row, 4, itemTrack);
                itemUID->setCheckState(Qt::Unchecked);
std::cout<<"           will trigger slot..."<<std::endl;
                _ressourcesTable->setItem(row, 0, itemUID);
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
	uint idx = _ui->axisSelection->currentIndex();
	uint dims[3] ;
	antHillMng.getSize( dims[1], dims[0],dims[2] ) ;
	_mainPix = QImage( dims[ (idx+1)%3], dims[ (idx+2)%3], QImage::Format_ARGB32);
	_ui->sequenceSlider->setMaximum( 0 );
	_ui->sequenceSlider->setMaximum( dims[idx]-1 );
	_ui->sequenceSlider->setSliderPosition(_currentSlice);
	
	_ui->sequenceSlider->setEnabled(true);
    /*
    _ui->spinMinIntensity->setEnabled(true);
	_ui->spinMaxIntensity->setEnabled(true);
    Interval<arma::s16> range ;
	antHillMng.getRange<arma::s16>( range ) ;//range.setBounds(0,5161);
	_ui->spinMinIntensity->setRange( range.min(), range.max() );
	_ui->spinMaxIntensity->setRange( range.min(), range.max() );
	_ui->spinMinIntensity->setValue( range.min() );
    _ui->spinMaxIntensity->setValue( range.max() );*/
	
	_ui->binSpinBox->setValue( 128 );
    updateDictionary( );
    updateRessources( ) ;

	drawSlice();
    /*
	_ui->segmLoadButton->setEnabled(true);
	_ui->skelLoadButton->setEnabled(true);
    */
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
    /**
      * \brief retrieve ressource's row devoted to input image
      */
    uint row = 0 ;
    while (  _ressourcesTable->item(row,0)->text() != antHillMng.inputuid() ) {
        row++ ;
    }
    QLineEdit *ql = qobject_cast< QLineEdit * >( _ressourcesTable->cellWidget(row,3) ) ;
    Interval<arma::s16> range_res = RANGE<arma::s16>( ql->text() ) ;
    antHillMng.binarization( range_res, _ui->binSpinBox->value()) ;
	updateRessources() ;
}

void MainWindow::on_checkBox_stateChanged(int arg1){
	_bViewSegm = !_bViewSegm ;
	drawSlice();
}
/*
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
*/
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
