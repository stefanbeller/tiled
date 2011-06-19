#include "projectactionhandler.h"

#include "project.h"
#include "projecttreemodel.h"
#include "projectview.h"
#include "utils.h"

#include <QObject>
#include <QAction>

#include <QDebug>
#include <QFileDialog>

using namespace Tiled;
using namespace Tiled::Internal;

ProjectActionHandler *ProjectActionHandler::mInstance = 0;

ProjectActionHandler::ProjectActionHandler(QObject *parent)
    : QObject(parent)
    , mActionNewProject(new QAction(this))
    , mActionNewMap(new QAction(this))
    , mActionAddMap(new QAction(this))
{
    Q_ASSERT(!mInstance);
    mInstance = this;
    mActionNewProject->setShortcut(tr("Ctrl+Shift+N"));

    connect(mActionNewProject, SIGNAL(triggered()), SLOT(newProject()));
    connect(mActionNewMap, SIGNAL(triggered()), SLOT(newMap()));
    connect(mActionAddMap, SIGNAL(triggered()), SLOT(addMap()));
    retranslateUi();
}

ProjectActionHandler::~ProjectActionHandler()
{
    mInstance = 0;
}

void ProjectActionHandler::retranslateUi()
{
    mActionNewProject->setText(tr("New Project"));
    mActionNewMap->setText(tr("New Map"));
    mActionAddMap->setText(tr("Add Map"));
}

void ProjectActionHandler::setProjectTreeModel(ProjectTreeModel *projectTreeModel)
{
    mProjectTreeModel = projectTreeModel;
}

void ProjectActionHandler::setProjectView(ProjectView *projectView)
{
    mProjectView = projectView;
}

void ProjectActionHandler::newProject()
{
    mProjectTreeModel->addProject(new Project(QString()));
}

void ProjectActionHandler::newMap()
{

}

void ProjectActionHandler::addMap()
{
    Project *project = mProjectView->selectedProject();;
    qDebug()<<"addMap to:"<<project;

//    QString filter = tr("All Files (*)");
//    filter += QLatin1String(";;");

//    QString selectedFilter = tr("Tiled map files (*.tmx)");
//    filter += selectedFilter;

//    //selectedFilter = QString();//mSettings.value(QLatin1String("lastUsedOpenFilter"),
//                                 //    selectedFilter).toString();

//    const QString startlocation=QString(tr(""));
//    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Map"),
//                                                    startlocation,
//                                                    filter, &selectedFilter);
//    if (fileNames.isEmpty())
//        return;

//    project->addMaps(fileNames);
}

