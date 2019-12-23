#include "stdf_viewer.h"
#include "ui_stdf_viewer.h"
#include <vector>
#include <stdlib.h>
#include <QInputDialog>
#include "string_utils.h"

using namespace std;

STDF_viewer::STDF_viewer(QWidget *parent)
    : QMainWindow(parent), records_per_page(10), page_number(0)
    , ui(new Ui::STDF_viewer)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(1);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->txtBusy->setVisible(false);
}

STDF_viewer::~STDF_viewer()
{
    delete ui;
}


void STDF_viewer::on_actionOpen_triggered()
{
    ui->txtBusy->setVisible(true);
    ui->txtBusy->repaint();
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Stdf"), "c:\\", tr("Stdf files (*.stdf)"));

    stdf_parser.stdf_parser_init(fileName.toStdString());
    QStringList rec_types;
    if (stdf_parser.get_stdf_parser_state() == 0) {
        headers = stdf_parser.get_all_headers();
        for (vector<header_t>::iterator it=headers.begin(); it != headers.end(); it++) {
            QString rec_type = QString::fromStdString(stdf_parser.get_rec_name(*it));
            if (!(rec_types.contains(rec_type))) {
                rec_types.append(rec_type);
            }
        }
        ui->listRecType->addItems(rec_types);
        refresh_display();
    }
    ui->txtBusy->setVisible(false);
}

void STDF_viewer::on_comboBox_currentTextChanged(const QString &arg1)
{
    if (stdf_parser.get_stdf_parser_state() == 0) {
        records_per_page = arg1.toInt();
        refresh_display();
    }
    ui->comboBox->setCurrentText( QString::number(records_per_page) );
}

void STDF_viewer::refresh_display()
{
    ui->txtBusy->setVisible(true);

    vector<string> records;
    int row_number=0;
    //Clear this so that we dont use too much memory.
    for (vector<QTableWidgetItem *>::iterator it=items.begin(); it != items.end(); it++) {
        delete *it;
    }
    items.clear();

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->clearContents();
    ui->tableWidget->clear();
    ui->editPages->setText(  QString::number(int(ceil(float(headers.size())/records_per_page))-1));



    for (int i=0; i<records_per_page; i++) {
        if ( (records_per_page*page_number + i) >= headers.size()) {
            break;
        }
        records.clear();
        stdf_parser.record_as_text(headers[records_per_page*page_number + i], records, true);

        for (vector<string>::iterator it=records.begin(); it != records.end(); it++) {
            bool match=false;
            string rec_type = stdf_parser.get_rec_name(headers[records_per_page*page_number + i]);
            string rec_data = *it;

            map<string, vector<string>>::iterator iter = text_filters.find(rec_type);
            if (iter != text_filters.end()) {
                if ( iter->second.size() == 0) {
                    match = true;
                } else {
                    for (int i=0; i<iter->second.size(); i++) {
                        if ( (rec_data).find( (iter->second)[i]) != string::npos) {
                            match = true;
                            break;
                        }
                    }
                }
            }
            if (!match) {
                map<string, vector<string>>::iterator iter_string_filters = text_filters.find("by_text");
                if (iter_string_filters != text_filters.end()) {
                    for (int i=0; i<(*iter_string_filters).second.size(); i++) {
                        if ( (rec_data).find( ((*iter_string_filters).second)[i]) != string::npos) {
                            match = true;
                            break;
                        }
                    }
                }
            }
            cout << *it << " " << rec_type << " " << match << " " << text_filters.size() << endl;
            if (!match && text_filters.size() > 0) {
                continue;
            }

            QTableWidgetItem *item = new QTableWidgetItem;
            items.push_back(item);
            ui->tableWidget->insertRow(row_number);
            item->setText(QString::fromStdString(*it));
            ui->tableWidget->setItem(row_number,0,item);
            row_number++;
        };
    }
    ui->tableWidget->scrollToTop();
    ui->txtBusy->setVisible(false);
}

void STDF_viewer::on_editPage_textChanged(const QString &arg1)
{
    if (stdf_parser.get_stdf_parser_state() == 0) {
        page_number = arg1.toInt();
        refresh_display();
    }
}

void STDF_viewer::on_editPage_editingFinished()
{

}

void STDF_viewer::on_labelNext_linkHovered(const QString &link)
{

}

void STDF_viewer::on_labelNext_linkActivated(const QString &link)
{
    page_number++;
    ui->editPage->setText(QString::number(page_number));
    refresh_display();
}

void STDF_viewer::on_labelPrev_linkActivated(const QString &link)
{
    if (page_number > 0) {
        page_number--;
        ui->editPage->setText(QString::number(page_number));
        refresh_display();
    }
}
void STDF_viewer::on_pushButton_pressed()
{

}

void STDF_viewer::on_buttonNext_pressed()
{
    if (page_number >= ui->editPages->text().toInt()) {
        return;
    }
    page_number++;
    ui->editPage->setText(QString::number(page_number));
    refresh_display();
}

void STDF_viewer::on_buttonPrev_pressed()
{
    if (page_number > 0) {
        page_number--;
        ui->editPage->setText(QString::number(page_number));
        refresh_display();
    }
}

void STDF_viewer::on_buttonFilter_pressed()
{
    cout << "Filtering" << endl;
    int start_rec_id = ui->lineStart->text().toInt();
    int end_rec_id = ui->lineEnd->text().toInt();
    string rec_types = "";

    text_filters.clear();
    for (int i=0; i<ui->listFilter->count(); i++) {
        string filter_by_rec_name = ui->listFilter->item(i)->text().toStdString();
        cout << filter_by_rec_name << endl;
        vector<string> filters = split(filter_by_rec_name, "=>");
        vector<string> text_filters_by_rec_name;
        if (filters.size() > 1 ) {
            text_filters_by_rec_name = split(filters[1],",");
        }
        text_filters[filters[0]] = text_filters_by_rec_name;
        if (rec_types == "") {
            rec_types += filters[0];
        } else {
            rec_types += "," + filters[0];
        }
    }
    cout << "rec_types obtained" << endl;

    string text_filter = ui->textFilter->toPlainText().toStdString();
    if ( text_filter != "") {
        cout << "text filter " << text_filter << endl;
        text_filters["by_text"] = vector<string>();
        vector<string> filters_by_text = split(text_filter, ",");
        for (vector<string>::iterator it=filters_by_text.begin(); it!=filters_by_text.end(); it++) {
            text_filters["by_text"].push_back(*it);
        }

    }
    cout << "text filters obtained" << endl;

    page_number = 0;
    ui->editPage->setText(QString::number(page_number));
    ui->txtBusy->setVisible(true);
    ui->txtBusy->repaint();

    map<string, vector<string>>::iterator it = text_filters.find("by_text");
    if (it != text_filters.end()) {
        cout << "Getting all rec_types" << endl;
        headers = stdf_parser.get_stdf_headers_of_type("", start_rec_id, end_rec_id);
    } else {
        cout << "Getting selected rec_types " << rec_types << endl;
        headers = stdf_parser.get_stdf_headers_of_type(rec_types, start_rec_id, end_rec_id);
    }

    cout << "headers obtained" << endl;

    //Remove headers that doesn't pass the filter
    vector<header_t> filtered_headers;
    for (vector<header_t>::iterator it=headers.begin(); it!=headers.end(); it++) {
        vector<string> records;
        stdf_parser.record_as_text(*it,records, true);
        string rec_type = stdf_parser.get_rec_name(*it);
        bool match = true;
        map<string, vector<string>>::iterator iterator = text_filters.find(rec_type);
        if (iterator != text_filters.end()) {
            if ( iterator->second.size() != 0) {
                for (int i=0; i<iterator->second.size(); i++) {
                    //check if all the filters found at least 1 match.
                    bool record_match = false;
                    for (int j=0; j<records.size(); j++) {
                        if (records[j].find( (iterator->second)[i]) != string::npos ) {
                            cout << "recorc match" << records[j] << " " << (iterator->second)[i] << endl;
                            record_match = true;
                            break;
                        }
                    }
                    if (!record_match) {
                        match = false;
                        break;
                    }
                }

            }
        } else {
            match = false;
        }
        if (match) {
            cout << it->rec_name << " matches" << it->rec_id << endl;
            filtered_headers.push_back(*it);
            continue;
        }

        iterator = text_filters.find("by_text");
        if (iterator != text_filters.end()) {
            for (int i=0; i<iterator->second.size(); i++ ){
                if (match) {
                    break;
                }
                for (int j=0; j<records.size(); j++) {
                    if (records[j].find(iterator->second.at(i)) != string::npos ) {
                        filtered_headers.push_back(*it);
                        match = true;
                        break;
                    }
                }
            }
        }
    }
    headers = filtered_headers;
    cout << "Refreshing page" << endl;
    ui->txtBusy->setVisible(false);
    refresh_display();
}

void STDF_viewer::on_buttonClear_pressed()
{
    text_filters.clear();
    ui->textFilter->clear();
    ui->txtBusy->setVisible(true);
    ui->txtBusy->repaint();
    QList<QListWidgetItem *> selected_items = ui->listRecType->selectedItems();
    for (QList<QListWidgetItem *>::iterator it=selected_items.begin(); it!=selected_items.end(); it++){
        ui->listRecType->setItemSelected(*it, false);
    }
    page_number = 0;
    headers = stdf_parser.get_all_headers();
    ui->editPage->setText(QString::number(page_number));
    ui->lineStart->setText(QString::number(-1));
    ui->lineEnd->setText(QString::number(-1));
    ui->txtBusy->setVisible(false);
    refresh_display();
}

void STDF_viewer::on_listRecType_itemDoubleClicked(QListWidgetItem *item)
{
    bool bOk;
    QString filter = QInputDialog::getText(0,
                 "Filter",
                 "Filter String(comma delimited):",
                 QLineEdit::Normal,
                 "",
                 &bOk);

    if (bOk) {
        QString filter_txt = item->text() + QString::fromStdString("=>") + filter;
        ui->listFilter->addItem(filter_txt);
    }

}

void STDF_viewer::on_listRecType_itemClicked(QListWidgetItem *item)
{

}

void STDF_viewer::on_listFilter_itemDoubleClicked(QListWidgetItem *item)
{
    ui->listFilter->removeItemWidget(item);
    delete item;
}

void STDF_viewer::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{

    //ui->tableWidget->resizeRowToContents(item->column());
    //ui->tableWidget->resizeRowToContents(item->row());
    //ui->tableWidget->setWordWrap(true);
}

void STDF_viewer::on_tableWidget_itemSelectionChanged()
{
    //ui->tableWidget->adjustSize();
    //ui->tableWidget->
}
