#ifndef PROJECTACTIONHANDLER_H
#define PROJECTACTIONHANDLER_H

#include <QObject>

class QAction;

namespace Tiled {
namespace Internal {

class MapDocument;
class ProjectTreeModel;
class ProjectView;

class ProjectActionHandler : public QObject
{
    Q_OBJECT

public:
    explicit ProjectActionHandler(QObject *parent = 0);
    ~ProjectActionHandler();

    static ProjectActionHandler *instance() { return mInstance; }

    void retranslateUi();

    void setProjectTreeModel(ProjectTreeModel *projectTreeModel);
    void setProjectView(ProjectView *projectView);

    QAction *actionNewProject() const { return mActionNewProject; }
    QAction *actionNewMap() const { return mActionNewMap; }
    QAction *actionAddMap() const { return mActionAddMap; }
signals:

private slots:
    void newProject();
    void newMap();
    void addMap();
private:
    QAction *mActionNewProject;
    QAction *mActionNewMap;
    QAction *mActionAddMap;

    static ProjectActionHandler *mInstance;
    ProjectView *mProjectView;
    ProjectTreeModel *mProjectTreeModel;
};

} // namespace Internal
} // namespace Tiled

#endif // PROJECTACTIONHANDLER_H
