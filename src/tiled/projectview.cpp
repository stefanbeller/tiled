#include "projectview.h"

#include "project.h"
#include "projecttreemodel.h"
#include "projectactionhandler.h"

#include <QDebug>

using namespace Tiled;
using namespace Tiled::Internal;

ProjectView::ProjectView(QWidget *parent)
    : QTreeView(parent)
{
    ProjectActionHandler *handler = ProjectActionHandler::instance();
    handler->setProjectView(this);

}



Project *ProjectView::selectedProject()
{
    Q_ASSERT(model());

    QList<QModelIndex> selInd = selectionModel()->selectedIndexes();
    qDebug()<<selInd;
    if (selInd.isEmpty())
        return 0;

    return static_cast<ProjectTreeModel*>(model())->getProjectByIndex(selInd.at(0));
}
