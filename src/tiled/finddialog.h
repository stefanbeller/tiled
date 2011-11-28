#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class QTreeView;
class QCheckBox;
class QVBoxLayout;
class QGraphicsView;
class QHBoxLayout;
class QSpacerItem;

namespace Tiled {

class Map;

namespace Internal {

class MapDocument;
class MapScene;
class MapView;

class StampBrush;

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(MapDocument *mapDocument, QWidget *parent = 0);
    ~FindDialog();

private slots:
    void clickedAdvancedSettings(bool enabled);

private:
    void setupUi(QDialog *FindDialog);
    void retranslateUi(QDialog *FindDialog);

    QWidget *mWidget;
    QVBoxLayout *mVerticalLayout;
    MapView *mView;
    MapScene *mScene;
    QCheckBox *mAdvancedSettings;
    QTreeView *mLayerSelect;
    QHBoxLayout *mButtons;
    QPushButton *mMarkButton;
    QSpacerItem *mHorizontalSpacer;
    QPushButton *mFindNextButton;
    QPushButton *mFindPreviousButton;



    MapScene *mMapScene;
    MapView *mMapView;
    QTreeView *mLayerSearch;

    Map *mMap; // my map to be displayed
    MapDocument *mMapDocument; // the document which should be searched

    StampBrush *mStampBrush;
};

} // namespace Internal
} // namespace Tiled

#endif // FINDDIALOG_H
