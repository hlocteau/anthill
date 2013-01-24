/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Thu Jan 24 19:51:51 2013
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSlider>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QTableWidget>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionOpen_folder;
    QAction *actionClose_folder;
    QAction *actionExit;
    QAction *actionOpen_project;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QLabel *SlicePosition;
    QSlider *sequenceSlider;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QHBoxLayout *horizontalLayout_2;
    QSpinBox *spinMinIntensity;
    QSpinBox *spinMaxIntensity;
    QVBoxLayout *verticalLayout_7;
    QCheckBox *checkBox;
    QSpinBox *binSpinBox;
    QPushButton *binPushButton;
    QLabel *infoLabel;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_6;
    QLabel *_labelSliceView;
    QVBoxLayout *verticalLayout_4;
    QListWidget *listWidget;
    QTableWidget *tableWidget;
    QHBoxLayout *horizontalLayout_4;
    QLabel *segmLabel;
    QCheckBox *segmCheckBox;
    QCheckBox *contentCheckBox;
    QSpinBox *x_shift;
    QSpinBox *y_shift;
    QSpinBox *z_shift;
    QPushButton *segmLoadButton;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_2;
    QCheckBox *skelCheckBox;
    QCheckBox *contentSkelCheckBox;
    QSpinBox *x_shift_skel;
    QSpinBox *y_shift_skel;
    QSpinBox *z_shift_skel;
    QPushButton *skelLoadButton;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(602, 573);
        actionOpen_folder = new QAction(MainWindow);
        actionOpen_folder->setObjectName(QString::fromUtf8("actionOpen_folder"));
        actionClose_folder = new QAction(MainWindow);
        actionClose_folder->setObjectName(QString::fromUtf8("actionClose_folder"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionOpen_project = new QAction(MainWindow);
        actionOpen_project->setObjectName(QString::fromUtf8("actionOpen_project"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        centralWidget->setMaximumSize(QSize(596, 507));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetMaximumSize);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetMaximumSize);
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, -1, 0, -1);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        SlicePosition = new QLabel(centralWidget);
        SlicePosition->setObjectName(QString::fromUtf8("SlicePosition"));

        verticalLayout_2->addWidget(SlicePosition);

        sequenceSlider = new QSlider(centralWidget);
        sequenceSlider->setObjectName(QString::fromUtf8("sequenceSlider"));
        sequenceSlider->setOrientation(Qt::Horizontal);

        verticalLayout_2->addWidget(sequenceSlider);


        horizontalLayout_3->addLayout(verticalLayout_2);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignCenter);

        verticalLayout_3->addWidget(label);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        spinMinIntensity = new QSpinBox(centralWidget);
        spinMinIntensity->setObjectName(QString::fromUtf8("spinMinIntensity"));

        horizontalLayout_2->addWidget(spinMinIntensity);

        spinMaxIntensity = new QSpinBox(centralWidget);
        spinMaxIntensity->setObjectName(QString::fromUtf8("spinMaxIntensity"));

        horizontalLayout_2->addWidget(spinMaxIntensity);


        verticalLayout_3->addLayout(horizontalLayout_2);


        horizontalLayout_3->addLayout(verticalLayout_3);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setSpacing(6);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        checkBox = new QCheckBox(centralWidget);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        verticalLayout_7->addWidget(checkBox);

        binSpinBox = new QSpinBox(centralWidget);
        binSpinBox->setObjectName(QString::fromUtf8("binSpinBox"));
        binSpinBox->setMaximum(255);

        verticalLayout_7->addWidget(binSpinBox);

        binPushButton = new QPushButton(centralWidget);
        binPushButton->setObjectName(QString::fromUtf8("binPushButton"));

        verticalLayout_7->addWidget(binPushButton);


        horizontalLayout_3->addLayout(verticalLayout_7);


        verticalLayout->addLayout(horizontalLayout_3);

        infoLabel = new QLabel(centralWidget);
        infoLabel->setObjectName(QString::fromUtf8("infoLabel"));

        verticalLayout->addWidget(infoLabel);

        scrollArea = new QScrollArea(centralWidget);
        scrollArea->setObjectName(QString::fromUtf8("scrollArea"));
        scrollArea->setMouseTracking(true);
        scrollArea->setContextMenuPolicy(Qt::NoContextMenu);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setFrameShadow(QFrame::Plain);
        scrollArea->setLineWidth(0);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignCenter);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 269, 351));
        verticalLayout_6 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_6->setSpacing(0);
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        _labelSliceView = new QLabel(scrollAreaWidgetContents);
        _labelSliceView->setObjectName(QString::fromUtf8("_labelSliceView"));
        _labelSliceView->setMouseTracking(true);
        _labelSliceView->setContextMenuPolicy(Qt::NoContextMenu);
        _labelSliceView->setLineWidth(0);
        _labelSliceView->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        verticalLayout_6->addWidget(_labelSliceView);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout->addWidget(scrollArea);


        horizontalLayout->addLayout(verticalLayout);


        gridLayout->addLayout(horizontalLayout, 4, 1, 1, 1);

        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        listWidget = new QListWidget(centralWidget);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        verticalLayout_4->addWidget(listWidget);

        tableWidget = new QTableWidget(centralWidget);
        if (tableWidget->columnCount() < 2)
            tableWidget->setColumnCount(2);
        if (tableWidget->rowCount() < 2)
            tableWidget->setRowCount(2);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setAlternatingRowColors(true);
        tableWidget->setRowCount(2);
        tableWidget->setColumnCount(2);
        tableWidget->verticalHeader()->setVisible(false);

        verticalLayout_4->addWidget(tableWidget);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        segmLabel = new QLabel(centralWidget);
        segmLabel->setObjectName(QString::fromUtf8("segmLabel"));

        horizontalLayout_4->addWidget(segmLabel);

        segmCheckBox = new QCheckBox(centralWidget);
        segmCheckBox->setObjectName(QString::fromUtf8("segmCheckBox"));
        segmCheckBox->setEnabled(false);
        segmCheckBox->setCheckable(true);
        segmCheckBox->setChecked(false);

        horizontalLayout_4->addWidget(segmCheckBox);

        contentCheckBox = new QCheckBox(centralWidget);
        contentCheckBox->setObjectName(QString::fromUtf8("contentCheckBox"));
        contentCheckBox->setEnabled(false);
        contentCheckBox->setTristate(false);

        horizontalLayout_4->addWidget(contentCheckBox);

        x_shift = new QSpinBox(centralWidget);
        x_shift->setObjectName(QString::fromUtf8("x_shift"));
        x_shift->setEnabled(false);

        horizontalLayout_4->addWidget(x_shift);

        y_shift = new QSpinBox(centralWidget);
        y_shift->setObjectName(QString::fromUtf8("y_shift"));
        y_shift->setEnabled(false);

        horizontalLayout_4->addWidget(y_shift);

        z_shift = new QSpinBox(centralWidget);
        z_shift->setObjectName(QString::fromUtf8("z_shift"));
        z_shift->setEnabled(false);

        horizontalLayout_4->addWidget(z_shift);

        segmLoadButton = new QPushButton(centralWidget);
        segmLoadButton->setObjectName(QString::fromUtf8("segmLoadButton"));
        segmLoadButton->setEnabled(false);

        horizontalLayout_4->addWidget(segmLoadButton);


        verticalLayout_4->addLayout(horizontalLayout_4);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_6->addWidget(label_2);

        skelCheckBox = new QCheckBox(centralWidget);
        skelCheckBox->setObjectName(QString::fromUtf8("skelCheckBox"));
        skelCheckBox->setEnabled(false);

        horizontalLayout_6->addWidget(skelCheckBox);

        contentSkelCheckBox = new QCheckBox(centralWidget);
        contentSkelCheckBox->setObjectName(QString::fromUtf8("contentSkelCheckBox"));
        contentSkelCheckBox->setEnabled(false);

        horizontalLayout_6->addWidget(contentSkelCheckBox);

        x_shift_skel = new QSpinBox(centralWidget);
        x_shift_skel->setObjectName(QString::fromUtf8("x_shift_skel"));
        x_shift_skel->setEnabled(false);

        horizontalLayout_6->addWidget(x_shift_skel);

        y_shift_skel = new QSpinBox(centralWidget);
        y_shift_skel->setObjectName(QString::fromUtf8("y_shift_skel"));
        y_shift_skel->setEnabled(false);

        horizontalLayout_6->addWidget(y_shift_skel);

        z_shift_skel = new QSpinBox(centralWidget);
        z_shift_skel->setObjectName(QString::fromUtf8("z_shift_skel"));
        z_shift_skel->setEnabled(false);

        horizontalLayout_6->addWidget(z_shift_skel);

        skelLoadButton = new QPushButton(centralWidget);
        skelLoadButton->setObjectName(QString::fromUtf8("skelLoadButton"));
        skelLoadButton->setEnabled(false);

        horizontalLayout_6->addWidget(skelLoadButton);


        verticalLayout_4->addLayout(horizontalLayout_6);


        gridLayout->addLayout(verticalLayout_4, 4, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 602, 25));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
#ifndef QT_NO_SHORTCUT
        SlicePosition->setBuddy(sequenceSlider);
        label->setBuddy(scrollArea);
#endif // QT_NO_SHORTCUT

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionOpen_project);
        menuFile->addAction(actionOpen_folder);
        menuFile->addAction(actionClose_folder);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "AntHouse GUI", 0, QApplication::UnicodeUTF8));
        actionOpen_folder->setText(QApplication::translate("MainWindow", "Import dicom folder", 0, QApplication::UnicodeUTF8));
        actionOpen_folder->setShortcut(QApplication::translate("MainWindow", "Ctrl+I", 0, QApplication::UnicodeUTF8));
        actionClose_folder->setText(QApplication::translate("MainWindow", "Close folder", 0, QApplication::UnicodeUTF8));
        actionClose_folder->setShortcut(QApplication::translate("MainWindow", "Ctrl+W", 0, QApplication::UnicodeUTF8));
        actionExit->setText(QApplication::translate("MainWindow", "Exit", 0, QApplication::UnicodeUTF8));
        actionExit->setShortcut(QApplication::translate("MainWindow", "Ctrl+X", 0, QApplication::UnicodeUTF8));
        actionOpen_project->setText(QApplication::translate("MainWindow", "Open project", 0, QApplication::UnicodeUTF8));
        actionOpen_project->setShortcut(QApplication::translate("MainWindow", "Ctrl+O, Ctrl+S", 0, QApplication::UnicodeUTF8));
        SlicePosition->setText(QApplication::translate("MainWindow", "Position", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Intensity", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("MainWindow", "Threshold", 0, QApplication::UnicodeUTF8));
        binPushButton->setText(QApplication::translate("MainWindow", "Run", 0, QApplication::UnicodeUTF8));
        infoLabel->setText(QApplication::translate("MainWindow", "Info", 0, QApplication::UnicodeUTF8));
        _labelSliceView->setText(QString());
        segmLabel->setText(QApplication::translate("MainWindow", "Segm.", 0, QApplication::UnicodeUTF8));
        segmCheckBox->setText(QApplication::translate("MainWindow", "View", 0, QApplication::UnicodeUTF8));
        contentCheckBox->setText(QApplication::translate("MainWindow", "content", 0, QApplication::UnicodeUTF8));
        segmLoadButton->setText(QApplication::translate("MainWindow", "Load...", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Skeleton", 0, QApplication::UnicodeUTF8));
        skelCheckBox->setText(QApplication::translate("MainWindow", "View", 0, QApplication::UnicodeUTF8));
        contentSkelCheckBox->setText(QApplication::translate("MainWindow", "content", 0, QApplication::UnicodeUTF8));
        skelLoadButton->setText(QApplication::translate("MainWindow", "Load", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
