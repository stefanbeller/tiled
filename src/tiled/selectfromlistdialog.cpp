#include "selectfromlistdialog.h"
#include "ui_selectfromlistdialog.h"

SelectFromListDialog::SelectFromListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectFromListDialog)
{
    ui->setupUi(this);
    ui->listView->setModel(new QStringListModel(ui->listView));
}

SelectFromListDialog::~SelectFromListDialog()
{
    delete ui;
}

QString SelectFromListDialog::getSelection()
{
    if (exec() != QDialog::Accepted)
        return QString();
    return ui->listView->currentIndex().data().toString();
}

void SelectFromListDialog::setStrings(QStringList list)
{
    reinterpret_cast<QStringListModel*>(ui->listView->model())->setStringList(list);
}
