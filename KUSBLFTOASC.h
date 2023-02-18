#pragma once

#include <QtWidgets/QWidget>
#include "ui_KUSBLFTOASC.h"
#include <qstringlist.h>
class KUSBLFTOASC : public QWidget
{
    Q_OBJECT

public:
    KUSBLFTOASC(QWidget *parent = nullptr);
    ~KUSBLFTOASC();

private:
    Ui::KUSBLFTOASCClass ui;
    QString m_sBlfName;
    QString m_sAscName;
    bool RunState = true;
    bool isFilter = false;
    QStringList fifter;

private:
    void runConveter();
signals:
    void sigStatus(int);
    void sigInit(int);
private slots:
    void on_pushButton_sleBlf_clicked();
    void on_pushButton_selAsc_clicked();
    void on_pushButton_converter_clicked();
    void on_recConveterState(int);
    void on_recInitState(int);

    virtual void closeEvent(QCloseEvent* event) override;

};
