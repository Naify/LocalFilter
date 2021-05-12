#include "localfilter.h"
#include "ui_localfilter.h"
#include <QDebug>
#include <QMessageBox>
#include "metaManager.h"
#include <QFileDialog>
#include "addeditfilter.h"

#if (QT_VERSION_MAJOR >= 5)
    #include <qjson-qt5/parser.h>
    #include <qjson-qt5/serializer.h>
#else
    #include <parser.h>
    #include <serializer.h>
#endif

LocalFilter::LocalFilter(const QString& SoftwareName, const QString& formId,
                         const QVariantList& Attrs, QWidget *parent) :
    QDialog(parent),
    softwareName(SoftwareName),
    formId(formId),
    attrList(Attrs),
    ui(new Ui::LocalFilter)
{
    ui->setupUi(this);
    init();
}

LocalFilter::~LocalFilter()
{
    delete ui;
}

void LocalFilter::init()
{
    setupTable();
    if (!loadFilters()){
        QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);
    }

    connect(ui->tv, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(edit()));
    connect(ui->pbAdd, SIGNAL(released()), this, SLOT(add()));
    connect(ui->pbEdit, SIGNAL(released()),this, SLOT(edit()));
    connect(ui->pbDel, SIGNAL(released()),this, SLOT(del()));
    connect(ui->pbSave, SIGNAL(released()),this, SLOT(save()));
    connect(ui->pbExit, SIGNAL(released()),this, SLOT(exit()));
}

void LocalFilter::setupTable()
{
    model = new QStandardItemModel();
    model->setHorizontalHeaderItem(0,new QStandardItem ("Атрибут"));
    model->setHorizontalHeaderItem(1,new QStandardItem ("Условие"));
    model->setHorizontalHeaderItem(2,new QStandardItem ("Значение"));

    ui->tv->setModel(model);
    ui->tv->resizeColumnsToContents();
    ui->tv->verticalHeader()->setVisible(false);
    ui->tv->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tv->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tv->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if (attrList.isEmpty()){
        QMessageBox::warning(0, "Внимание", "Отсутствуют поля для фильтрации\n"
                                            "Добавление и измение фильтров недоступно");
        disconnect(ui->tv, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(edit()));
        ui->pbAdd->setEnabled(false);
        ui->pbEdit->setEnabled(false);
        ui->pbDel->setEnabled(false);
        ui->pbSave->setEnabled(false);
    }
}

bool LocalFilter::makeJson()
{
    QVariantList root;

    fillJson(root);

    QJson::Serializer serializer;
//    serializer.setIndentMode(QJson::IndentFull);
    serializer.setIndentMode(QJson::IndentCompact);

    QByteArray json = serializer.serialize(root);
    if( json.isEmpty() ) {
        QMessageBox::warning(0, "Предупреждение",
        QString("Не удалось сформировать файл json:\n%1")
        .arg(serializer.errorMessage()));
        return false;
    }

    MetaManager mm = MetaManager();
    SharedSoftware currentTask = mm.getSoftware( softwareName );

    if( currentTask.isNull() ) {
        QMessageBox::warning(0, "Предупреждение", "Не найдена задача");
        return false;
    }

    SharedMetaForm form = currentTask->getFormByFormId(formId);

    if (!form->isValid()){
        QMessageBox::warning(0, "Предупреждение", "Форма недействительна");
        return false;
    }

    form->setJSONFilter(QString(json));

    if (currentTask->saveForm(form))
        return true;
    else
        return false;

//    QString filename = QFileDialog::getSaveFileName(this,
//    "Экспорт файла", "/home/user/projects/LocalFilter/exportTest.json", "*.json");
//    if( filename.isEmpty() ) {
//        QMessageBox::warning(0, "Предупреждение",
//        QString("Не задано имя файла"));
//        return false;
//    }

//    if( !filename.endsWith(".json") ) filename.append(".json");
//    QFile file(filename);
//    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
//    QTextStream out(&file);
//    out << json;
//    file.close();
//    return true;
}

bool LocalFilter::parseJsonAddfilters(const QByteArray &data)
{
    QJson::Parser parser; bool ok;
    QVariantList root = parser.parse(data, &ok).toList();
    if( !ok ){
        QMessageBox::warning(0, "Предупреждение",
                             QString("Не удалось обработать файл json:\nСтрока %1\n%2")
                             .arg(parser.errorLine()).arg(parser.errorString()));
        return false;
    }

    foreach (QVariant obj, root) {
        QVariantMap map = obj.toMap();
        QList<QStandardItem*> itemList;

        QStandardItem* item = new QStandardItem;
        item->setText(map["name"].toString());
        QString type = map["type"].toString();
        item->setData(type, 20);
        if (type == "Link")
            item->setData(map["linkVal"].toString(), 21);
        item->setData(map["table"].toString(), 22);
        itemList<<item;
        item = new QStandardItem;
        item->setText(map["sign"].toString());
        item->setTextAlignment(Qt::AlignCenter);
        itemList<<item;
        item = new QStandardItem;
        item->setText(map["value"].toString());
        itemList<<item;

        model->appendRow(itemList);
    }

    ui->tv->setCurrentIndex(QModelIndex());
    return true;
}

bool LocalFilter::loadFilters()
{
    if (softwareName.isEmpty() ){
        QMessageBox::warning(0, "Внимание", "Получено пустое название задачи\n"
                                            "Загрузка фильтров невозможна");
        return false;
    } else if  (formId.isEmpty()){
        QMessageBox::warning(0, "Внимание", "Получен пустой идентификатор формы\n"
                                            "Загрузка фильтров невозможна");
        return false;
    }

    if (!ODS::OdsInterface::self()->connectionManager().isConnected()){
        QMessageBox::warning(0, "Внимание", "Нет подключения к БД\n"
                                            "Загрузка фильтров невозможна");
        return false;
    }

    MetaManager mm = MetaManager();
    SharedSoftware currentTask = mm.getSoftware( softwareName );

    if( currentTask.isNull() ) {
        QMessageBox::warning(0, "Предупреждение", "Не найдена задача\n"
                                                  "Загрузка фильтров невозможна");
        return false;
    }

    SharedMetaForm form = currentTask->getFormByFormId(formId);

    if (!form->isValid()){
        QMessageBox::warning(0, "Предупреждение", "Форма недействительна\n"
                                                  "Загрузка фильтров невозможна");
        return false;
    }

    QString json = form->JSONFilter();

    if (!json.isEmpty()){
        QByteArray data = json.toUtf8();
        return parseJsonAddfilters(data);
    } else
        return true;

//    QString json = QFileDialog::getOpenFileName(this,
//                        "Выберите файл",
//                        "/home/user/projects/LocalFilter/exportTest.json",
//                        "*.json");

//    QFile file;
//    if( !json.isEmpty() ){
//        file.setFileName(json);
//    }else{
//        qDebug()<<"Empty! =(";
//        return;
//    }

//    if( !file.open( QIODevice::ReadOnly) ){
//        QMessageBox::warning(this, "Внимание", "Не удалось открыть json на чтение ");
//    }

//    QByteArray data = file.readAll();
}

void LocalFilter::fillJson(QVariantList &root)
{
    QStringList strList;
    strList<<"name"<<"sign"<<"value";
    int row = model->rowCount();
    int col = model->columnCount();
    for (int i = 0; i < row ; ++i){
        QVariantMap condition;
        for (int j = 0; j < col; j++){
            QString content = model->data(model->index(i, j),
                                           Qt::DisplayRole).toString();
            condition.insert(strList[j], content);
            QString type = model->data(model->index(i, 0), 20).toString();
            condition.insert("type", type);
            if (type == "Link"){
                QString linkVal = model->data(model->index(i, 0), 21).toString();
                condition.insert("linkVal", linkVal);
            }
            condition.insert("table", model->data(model->index(i, 0), 22).toString());
        }
        root << condition;
    }    
}

void LocalFilter::save()
{
    if (makeJson())
        this->accept();
}

void LocalFilter::add()
{
    AddEditFilter* addFilter = new AddEditFilter(attrList);
    connect(addFilter, SIGNAL(sendNewCondition(QMap<QString,QString>)),
            this, SLOT(addCondition(QMap<QString,QString>)));
    addFilter->exec();
}

void LocalFilter::edit()
{
    if (ui->tv->currentIndex().row() == -1){
        QMessageBox::warning(this,"Внимание","Сначала выберите ряд");
    } else {
        QItemSelection selection = ui->tv->selectionModel()->selection();
        QStringList strList;
        foreach (QModelIndex i, selection.indexes()) {
            strList<<i.data().toString();
        }
        strList<<QString::number(selection.indexes().at(0).row());
        AddEditFilter* editFilter = new AddEditFilter(strList, attrList);
        connect(editFilter, SIGNAL(sendNewCondition(QMap<QString,QString>)),
                this, SLOT(addCondition(QMap<QString,QString>)));
        editFilter->exec();
    }
}

void LocalFilter::del()
{
    if (ui->tv->currentIndex().row() == -1){
        QMessageBox::warning(this,"Внимание","Сначала выберите ряд");
    } else {
        ui->tv->model()->removeRow(ui->tv->currentIndex().row());
    }
}

void LocalFilter::exit()
{
    this->reject();
}

void LocalFilter::addCondition(QMap<QString,QString> newRow)
{
    QList<QStandardItem*> itemList;

    if (newRow.contains("value2")){

        //если выбрано условие "от"
        int rowNumber = newRow["rowIndex"].toInt();

        if (rowNumber != -1){            
            //не новый ряд
            QList<QStandardItem*> itemRow = model->takeRow(rowNumber);
            itemRow.at(0)->setText(newRow["attr"]);
            QString type = newRow["type"];
            itemRow.at(0)->setData(type, 20);
            if (type == "Link")
                itemRow.at(0)->setData(newRow["linkVal"], 21);
            itemRow.at(0)->setData(newRow["table"], 22);
            itemRow.at(1)->setText(newRow["sign"]);
            QString changedText = newRow["value"].append(" до ").append(newRow["value2"]);
            itemRow.at(2)->setText(changedText);

            model->insertRow(rowNumber, itemRow);

        } else {

            QStandardItem* item = new QStandardItem;
            item->setText(newRow["attr"]);
            QString type = newRow["type"];
            item->setData(type, 20);
            if (type == "Link")
                item->setData(newRow["linkVal"], 21);
            item->setData(newRow["table"], 22);
            itemList<<item;
            item = new QStandardItem;
            item->setText(newRow["sign"]);
            item->setTextAlignment(Qt::AlignCenter);
            itemList<<item;
            item = new QStandardItem;
            item->setText(newRow["value"]+" до "+newRow["value2"]);
            itemList<<item;
            model->appendRow(itemList);
        }

    } else {

        // не "от"
        int rowNumber = newRow["rowIndex"].toInt();

        if (rowNumber != -1){

            //не новый ряд
            QList<QStandardItem*> itemRow = model->takeRow(rowNumber);
            itemRow.at(0)->setText(newRow["attr"]);
            QString type = newRow["type"];
            itemRow.at(0)->setData(type, 20);
            if (type == "Link")
                itemRow.at(0)->setData(newRow["linkVal"], 21);
            itemRow.at(0)->setData(newRow["table"], 22);
            itemRow.at(1)->setText(newRow["sign"]);
            itemRow.at(2)->setText(newRow["value"]);
            model->insertRow(rowNumber,itemRow);

        } else {
            QStandardItem* item = new QStandardItem;
            item->setText(newRow["attr"]);
            QString type = newRow["type"];
            item->setData(type, 20);
            if (type == "Link")
                item->setData(newRow["linkVal"], 21);
            item->setData(newRow["table"], 22);
            itemList<<item;
            item = new QStandardItem;
            item->setText(newRow["sign"]);
            item->setTextAlignment(Qt::AlignCenter);
            itemList<<item;
            item = new QStandardItem;
            item->setText(newRow["value"]);
            itemList<<item;
            model->appendRow(itemList);
        }
    }

}
