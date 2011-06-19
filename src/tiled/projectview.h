#ifndef PROJECTSVIEW_H
#define PROJECTSVIEW_H

#include <QTreeView>

namespace Tiled {
namespace Internal {

class Project;
class ProjectTreeModel;

class ProjectView : public QTreeView
{
public:
    ProjectView(QWidget *parent);
    Project *selectedProject();

};

} // namespace Tiled::Internal
} // namespace Tiled

#endif // PROJECTSVIEW_H
