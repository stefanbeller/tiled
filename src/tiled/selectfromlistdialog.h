#ifndef SELECTFROMLISTDIALOG_H
#define SELECTFROMLISTDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QStringListModel>


namespace Ui {
    class SelectFromListDialog;
}

class SelectFromListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectFromListDialog(QWidget *parent = 0);
    ~SelectFromListDialog();
    QString getSelection();

public slots:
    void setStrings(QStringList list);

private:
    Ui::SelectFromListDialog *ui;
};

#endif // SELECTFROMLISTDIALOG_H
