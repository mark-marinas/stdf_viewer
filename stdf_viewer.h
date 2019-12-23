#ifndef STDF_VIEWER_H
#define STDF_VIEWER_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidgetItem>

#include <string>
#include <QListWidgetItem>
#include "Stdf_parser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class STDF_viewer; }
QT_END_NAMESPACE

class STDF_viewer : public QMainWindow
{
    Q_OBJECT
public:
    Stdf_parser stdf_parser;
    STDF_viewer(QWidget *parent = nullptr);
    ~STDF_viewer();
private slots:
    void on_actionOpen_triggered();

    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_editPage_textChanged(const QString &arg1);

    void on_editPage_editingFinished();

    void on_labelNext_linkHovered(const QString &link);

    void on_labelNext_linkActivated(const QString &link);

    void on_labelPrev_linkActivated(const QString &link);

    void on_pushButton_pressed();

    void on_buttonPrev_pressed();

    void on_buttonNext_pressed();

    void on_buttonFilter_pressed();

    void on_buttonClear_pressed();

    void on_listRecType_itemDoubleClicked(QListWidgetItem *item);

    void on_listRecType_itemClicked(QListWidgetItem *item);

    void on_listFilter_itemDoubleClicked(QListWidgetItem *item);

    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

    void on_tableWidget_itemSelectionChanged();

private:
    Ui::STDF_viewer *ui;
    int records_per_page;
    int page_number;
    map<string, vector<string>> text_filters;
    vector<header_t> headers;
    vector<QTableWidgetItem *> items;
    void refresh_display();

};
#endif // STDF_VIEWER_H
