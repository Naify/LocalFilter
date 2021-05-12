#ifndef ADDEDITFILTER_H
#define ADDEDITFILTER_H

#include <QDialog>
#include <QMap>

namespace Ui {
class AddEditFilter;
}

class AddEditFilter : public QDialog
{
    Q_OBJECT

public:
    explicit AddEditFilter(const QVariantList& attrList, QWidget *parent = 0);
    explicit AddEditFilter(const QStringList& strList,
                           const QVariantList& attrList,
                           QWidget *parent = 0);
    ~AddEditFilter();

private:
    void init(const QVariantList& attrList);
    const int editRowNumber;
    Ui::AddEditFilter *ui;
    bool checkIsLink(const QString &attr);

private slots:
    void attrChanged(QString attrName);
    void conditionChanged(QString condition);
    void saveCondition();
    void exit();

signals:
    void sendNewCondition(QMap<QString,QString>);
};

#endif // ADDEDITFILTER_H
