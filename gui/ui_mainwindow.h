/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Fri Feb 15 17:10:37 2013
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
#include <QtGui/QComboBox>
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
#include <QtGui/QToolBox>
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
    QHBoxLayout *horizontalLayout_7;
    QVBoxLayout *verticalLayout_4;
    QListWidget *listWidget;
    QToolBox *toolBox;
    QWidget *dictionary;
    QHBoxLayout *horizontalLayout_5;
    QTableWidget *tableWidget;
    QWidget *ressources;
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QVBoxLayout *verticalLayout_2;
    QComboBox *axisSelection;
    QLabel *SlicePosition;
    QSlider *sequenceSlider;
    QVBoxLayout *verticalLayout_7;
    QCheckBox *checkBox;
    QSpinBox *binSpinBox;
    QPushButton *binPushButton;
    QLabel *infoLabel;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_6;
    QLabel *_labelSliceView;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(807, 573);
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
        centralWidget->setMaximumSize(QSize(805, 507));
        horizontalLayout_7 = new QHBoxLayout(centralWidget);
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalLayout_7->setSizeConstraint(QLayout::SetMaximumSize);
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        listWidget = new QListWidget(centralWidget);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));

        verticalLayout_4->addWidget(listWidget);

        toolBox = new QToolBox(centralWidget);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        dictionary = new QWidget();
        dictionary->setObjectName(QString::fromUtf8("dictionary"));
        dictionary->setGeometry(QRect(0, 0, 98, 96));
        horizontalLayout_5 = new QHBoxLayout(dictionary);
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        tableWidget = new QTableWidget(dictionary);
        if (tableWidget->columnCount() < 2)
            tableWidget->setColumnCount(2);
        if (tableWidget->rowCount() < 2)
            tableWidget->setRowCount(2);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setAlternatingRowColors(true);
        tableWidget->setRowCount(2);
        tableWidget->setColumnCount(2);
        tableWidget->verticalHeader()->setVisible(false);

        horizontalLayout_5->addWidget(tableWidget);

        toolBox->addItem(dictionary, QString::fromUtf8("Dictionary"));
        ressources = new QWidget();
        ressources->setObjectName(QString::fromUtf8("ressources"));
        ressources->setGeometry(QRect(0, 0, 389, 178));
        toolBox->addItem(ressources, QString::fromUtf8("Ressources"));

        verticalLayout_4->addWidget(toolBox);


        horizontalLayout_7->addLayout(verticalLayout_4);

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
        axisSelection = new QComboBox(centralWidget);
        axisSelection->setObjectName(QString::fromUtf8("axisSelection"));

        verticalLayout_2->addWidget(axisSelection);

        SlicePosition = new QLabel(centralWidget);
        SlicePosition->setObjectName(QString::fromUtf8("SlicePosition"));

        verticalLayout_2->addWidget(SlicePosition);

        sequenceSlider = new QSlider(centralWidget);
        sequenceSlider->setObjectName(QString::fromUtf8("sequenceSlider"));
        sequenceSlider->setOrientation(Qt::Horizontal);

        verticalLayout_2->addWidget(sequenceSlider);


        horizontalLayout_3->addLayout(verticalLayout_2);

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
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 373, 351));
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


        horizontalLayout_7->addLayout(horizontalLayout);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 807, 25));
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
#endif // QT_NO_SHORTCUT

        menuBar->addAction(menuFile->menuAction());
        menuFile->addAction(actionOpen_project);
        menuFile->addAction(actionOpen_folder);
        menuFile->addAction(actionClose_folder);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);

        retranslateUi(MainWindow);

        toolBox->setCurrentIndex(1);


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
        actionOpen_project->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(dictionary), QApplication::translate("MainWindow", "Dictionary", 0, QApplication::UnicodeUTF8));
        toolBox->setItemText(toolBox->indexOf(ressources), QApplication::translate("MainWindow", "Ressources", 0, QApplication::UnicodeUTF8));
        SlicePosition->setText(QApplication::translate("MainWindow", "Position", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("MainWindow", "Threshold", 0, QApplication::UnicodeUTF8));
        binPushButton->setText(QApplication::translate("MainWindow", "Run", 0, QApplication::UnicodeUTF8));
        infoLabel->setText(QApplication::translate("MainWindow", "Info", 0, QApplication::UnicodeUTF8));
        _labelSliceView->setText(QString());
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
