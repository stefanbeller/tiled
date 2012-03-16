#include "finddialog.h"
#include "ui_finddialog.h"

#include "map.h"
#include "mapscene.h"
#include "mapview.h"
#include "mapdocument.h"

#include "stampbrush.h"
#include "documentmanager.h"
#include "zoomable.h"

#include "toolmanager.h"

#include <QTreeView>

using namespace Tiled;
using namespace Tiled::Internal;

FindDialog::FindDialog(MapDocument *document, QWidget *parent)
    : QDialog(parent)
    //, mMapView(new MapView())
    , mMapDocument(document)
    , mStampBrush(new StampBrush())
{
    setupUi(this);

    if (StampBrush *tool = dynamic_cast<StampBrush*>(ToolManager::instance()->selectedTool()))
        mStampBrush->setStamp(tool->stamp());
}

void FindDialog::setupUi(QDialog *FindDialog)
{
    FindDialog->resize(451, 311);
    FindDialog->setAcceptDrops(false);
    mWidget = new QWidget(FindDialog);
    mWidget->setGeometry(QRect(-1, -1, 451, 311));
    mVerticalLayout = new QVBoxLayout(mWidget);
    mVerticalLayout->setSizeConstraint(QLayout::SetNoConstraint);
    mVerticalLayout->setContentsMargins(0, 0, 0, 0);

    mView = new MapView(mWidget);
    mScene = new MapScene(mView); // scene is owned by the view

    Map *map = mMapDocument->map();
    Map::Orientation orientation = map->orientation();
    int tileHeight = map->tileHeight();
    int tileWidth = map->tileWidth();

    mMap = new Map(orientation, 5, 5, tileWidth, tileHeight);
    MapDocument *md = new MapDocument(mMap);
    mScene->setMapDocument(md);
    mView->setScene(mScene);
    //mStampBrush->setMapDocument(md);
    mStampBrush->activate(mScene);

    mVerticalLayout->addWidget(mView);

    mAdvancedSettings = new QCheckBox(mWidget);
    connect(mAdvancedSettings, SIGNAL(clicked(bool)),
            this, SLOT(clickedAdvancedSettings(bool)));

    mVerticalLayout->addWidget(mAdvancedSettings);

    mLayerSelect = new QTreeView(mWidget);

    mVerticalLayout->addWidget(mLayerSelect);

    mButtons = new QHBoxLayout();
    mMarkButton = new QPushButton(mWidget);

    mButtons->addWidget(mMarkButton);

    mHorizontalSpacer = new QSpacerItem(40, 20,
                                        QSizePolicy::Expanding,
                                        QSizePolicy::Minimum);

    mButtons->addItem(mHorizontalSpacer);

    mFindNextButton = new QPushButton(mWidget);

    mButtons->addWidget(mFindNextButton);

    mFindPreviousButton = new QPushButton(mWidget);

    mButtons->addWidget(mFindPreviousButton);

    mVerticalLayout->addLayout(mButtons);

    retranslateUi(FindDialog);

    QMetaObject::connectSlotsByName(FindDialog);
    clickedAdvancedSettings(mAdvancedSettings->checkState() == Qt::Checked);
} // setupUi

void FindDialog::retranslateUi(QDialog *FindDialog)
{
    FindDialog->setWindowTitle(QApplication::translate("FindDialog", "FindDialog", 0, QApplication::UnicodeUTF8));
    mAdvancedSettings->setText(QApplication::translate("FindDialog", "advanced Settings", 0, QApplication::UnicodeUTF8));
    mMarkButton->setText(QApplication::translate("FindDialog", "Mark", 0, QApplication::UnicodeUTF8));
    mFindNextButton->setText(QApplication::translate("FindDialog", "Find Next", 0, QApplication::UnicodeUTF8));
    mFindPreviousButton->setText(QApplication::translate("FindDialog", "Find Previous", 0, QApplication::UnicodeUTF8));
}

void FindDialog::clickedAdvancedSettings(bool enabled)
{
    if (enabled) {
        mVerticalLayout->insertWidget(2, mLayerSelect);
        mLayerSelect->show();
    } else {
        mVerticalLayout->removeWidget(mLayerSelect);
        mLayerSelect->hide();
    }
    mVerticalLayout->update();
    this->update();
}

FindDialog::~FindDialog()
{

}
