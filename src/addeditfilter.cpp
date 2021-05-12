#include "addeditfilter.h"
#include "ui_addeditfilter.h"
#include <QDebug>
#include <QLabel>
#include <CatalogManager>
#include <QMessageBox>

AddEditFilter::AddEditFilter(const QVariantList& attrList, QWidget *parent) :
    QDialog(parent),
    editRowNumber(-1),
    ui(new Ui::AddEditFilter)
{
    ui->setupUi(this);
    init(attrList);

    connect(ui->cbAttr, SIGNAL(currentIndexChanged(QString)), this, SLOT(attrChanged(QString)));
    connect(ui->cbCondition, SIGNAL(currentIndexChanged(QString)), this, SLOT(conditionChanged(QString)));

    attrChanged(ui->cbAttr->currentText());
}

AddEditFilter::AddEditFilter(const QStringList& strList, const QVariantList& attrList, QWidget *parent) :
    QDialog(parent),
    editRowNumber(strList.at(3).toInt()),
    ui(new Ui::AddEditFilter)
{
    ui->setupUi(this);

    init(attrList);

    connect(ui->cbAttr, SIGNAL(currentIndexChanged(QString)), this, SLOT(attrChanged(QString)));
    connect(ui->cbCondition, SIGNAL(currentIndexChanged(QString)), this, SLOT(conditionChanged(QString)));

    ui->cbAttr->setCurrentIndex(ui->cbAttr->findText(strList.at(0)));
    attrChanged(ui->cbAttr->currentText());
    ui->cbCondition->setCurrentIndex(ui->cbCondition->findText(strList.at(1)));
    conditionChanged(ui->cbCondition->currentText());

    if (strList.at(1) == "от"){

        QStringList fromToConditions = strList.at(2).split(" до ");

        ui->leValue->setText(fromToConditions.at(0));
        ui->leValue2->setText(fromToConditions.at(1));
    } else {

        if (checkIsLink(strList.at(0))){
            ui->cbLinkValue->setCurrentText(strList.at(2));
        } else {
            ui->leValue->setText(strList.at(2));
        }
    }
}

AddEditFilter::~AddEditFilter()
{
    delete ui;
}

void AddEditFilter::init(const QVariantList& attrList)
{
    foreach (auto obj, attrList) {
        QVariantMap map = obj.toMap();
        ui->cbAttr->addItem(map["name"].toString(), map);
    }

    connect(ui->pbSave, SIGNAL(released()),this, SLOT(saveCondition()));
    connect(ui->pbExit, SIGNAL(released()),this, SLOT(exit()));
}

bool AddEditFilter::checkIsLink(const QString& attr)
{
    if( attr.isNull() )
        return false;

    QString type = ui->cbAttr->itemData(ui->cbAttr->findText(attr)).toMap()["type"].toString();

    if (type != "String" && type != "Int" && type != "Date"){
        return true;
    } else
        return false;
}

void AddEditFilter::attrChanged(QString attrName)
{
    QString attrType = ui->cbAttr->itemData(ui->cbAttr->findText(attrName)).toMap()["type"].toString();

    ui->cbCondition->clear();

    if (attrType == "String"){
        ui->cbCondition->addItem("=");
        ui->cbCondition->addItem("≠");
        ui->cbCondition->addItem("∈");

        ui->leValue->clear();
        ui->leValue->setValidator(0);
        ui->leValue->setPlaceholderText("");

    } else if (attrType == "Int"){
        ui->cbCondition->addItem("=");
        ui->cbCondition->addItem(">");
        ui->cbCondition->addItem("<");
        ui->cbCondition->addItem(">=");
        ui->cbCondition->addItem("<=");
        ui->cbCondition->addItem("≠");
        ui->cbCondition->addItem("от");

        ui->leValue->clear();
        ui->leValue2->clear();
        QIntValidator* validator = new QIntValidator();
        ui->leValue->setValidator(validator);
        ui->leValue->setPlaceholderText("");
        ui->leValue2->setValidator(validator);
        ui->leValue2->setPlaceholderText("");

    } else if (attrType == "Date"){
        ui->cbCondition->addItem("=");
        ui->cbCondition->addItem(">");
        ui->cbCondition->addItem("<");
        ui->cbCondition->addItem(">=");
        ui->cbCondition->addItem("<=");
        ui->cbCondition->addItem("≠");
        ui->cbCondition->addItem("от");

        ui->leValue->clear();
        ui->leValue2->clear();
        QRegExpValidator* validator =
                new QRegExpValidator(QRegExp("^[0-9]{2}\\.[0-9]{2}\\.[0-9]{4}$"));
        ui->leValue->setValidator(validator);
        ui->leValue->setPlaceholderText("06.07.2020");
        ui->leValue2->setValidator(validator);
        ui->leValue2->setPlaceholderText("06.07.2020");

    } else {
        //if link
        ui->cbCondition->addItem("=");

        ui->cbLinkValue->clear();

        IRS::CatalogManager catalogM = IRS::CatalogManager(ODS::OdsInterface::self());
        QVariantMap map = ui->cbAttr->currentData().toMap();

        QString str = map["linkVal"].toString();

        QStringList attrSplit;
        attrSplit= str.split("\"");

        attrSplit.removeAll("");

        if (attrSplit.size() <= 2){
            QMessageBox::warning(0, "Внимание", "Неизвестен атрибут для получения записей из словаря\n"
                                                "Невозможно загрузить словарные значения");
            ui->cbLinkValue->setEnabled(false);
        } else {

            IRS::Catalog::ptr catalog = catalogM.getCatalogByName(attrSplit.at(0));
            if (catalog.isNull()){
                QMessageBox::warning(0, "Внимание", "Не удалось получить действительный каталог\n"
                                                    "Невозможно загрузить словарные значения");
                ui->cbLinkValue->setEnabled(false);
                return;
            }
            IRS::CatalogEntry::ptrList entries = catalog->entries();

            foreach (IRS::CatalogEntry::ptr entry, entries) {

                if (entry.isNull())
                        continue;
                QString str = entry->stringValue(attrSplit.at(2));
                ui->cbLinkValue->addItem(str);
            }
        }
    }
}

void AddEditFilter::conditionChanged(QString condition)
{
    const static int  wWidth = this->width();

    if (condition == "от"){

        this->resize(wWidth+200, this->height());

        ui->leValue->show();
        ui->lbTo->show();
        ui->leValue2->show();
        ui->cbLinkValue->hide();

    } else {

        this->resize(wWidth, this->height());

        if (checkIsLink(ui->cbAttr->currentText())){

            ui->leValue->hide();
            ui->lbTo->hide();
            ui->leValue2->hide();
            ui->cbLinkValue->show();

        } else {

            ui->leValue->show();
            ui->lbTo->hide();
            ui->leValue2->hide();
            ui->cbLinkValue->hide();
        }
    }
}

void AddEditFilter::saveCondition()
{
    QVariantMap dataMap = ui->cbAttr->currentData().toMap();    
    QString attrType = ui->cbAttr->currentData().toMap()["type"].toString();

    QMap<QString,QString> newRow;

    newRow.insert("type", attrType);
    newRow.insert("attr", ui->cbAttr->currentText());
    newRow.insert("table", dataMap["table"].toString());
    newRow.insert("sign", ui->cbCondition->currentText());
    newRow.insert("rowIndex", QString::number(editRowNumber));

    if (attrType == "Link") {
        if (ui->cbLinkValue->isEnabled()){
            newRow.insert("value", ui->cbLinkValue->currentText());
        } else {
            QMessageBox::warning(0, "Внимание", "Неизвестен атрибут для получения записей из словаря\n"
                                                "Невозможно сохранить запись.");
            return;
        }
        newRow.insert("linkVal", dataMap["linkVal"].toString());        
    } else {

        if (ui->leValue->text().isEmpty()){
            QMessageBox::warning(0, "Внимание", "Не введено значение для сохранения\n"
                                                "Невозможно сохранить запись.");
            return;
        } else {
            newRow.insert("value", ui->leValue->text());
        }

        if (ui->cbCondition->currentText() == "от"){

            if (ui->leValue2->text().isEmpty()){
                QMessageBox::warning(0, "Внимание", "Не введено второе значение для сохранения\n"
                                                    "Невозможно сохранить запись.");
                return;
            } else {
                newRow.insert("value2", ui->leValue2->text());
            }
        }
    }

    emit sendNewCondition(newRow);
    this->accept();
}

void AddEditFilter::exit()
{
    this->reject();
}
