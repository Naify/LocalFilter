#ifndef LOCALFILTER_H
#define LOCALFILTER_H

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class LocalFilter;
}

class LocalFilter : public QDialog
{
    Q_OBJECT

public:
    explicit LocalFilter(const QString &SoftwareName, const QString &formId,
                          const QVariantList &Attrs, QWidget *parent = 0);
    ~LocalFilter();

private:    
    const QString softwareName;
    const QString formId;
    const QVariantList attrList;
    Ui::LocalFilter *ui;
    QStandardItemModel* model;

    void init();
    void setupTable();
    bool makeJson();
    bool parseJsonAddfilters(const QByteArray& data);
    bool loadFilters();
    void fillJson(QVariantList &root);

private slots:
    void save();
    void add();
    void edit();
    void del();
    void exit();
    void addCondition(QMap<QString, QString> newRow);
};

#endif // LOCALFILTER_H
