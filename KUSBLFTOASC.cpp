#include "KUSBLFTOASC.h"
#include <qfiledialog.h>
#include <qstring.h>
#include <qapplication.h>
#define STRICT                         /* WIN32 */
#include <windows.h>
#include "binlog.h"                    /* BL    */
#include <tchar.h>                     /* RTL   */
#include <qtextstream.h>
#include <QFile>
#include <QtConcurrent>
#include <qdebug.h>
#include <qmessagebox.h>
#include <qicon.h>
# pragma execution_character_set("utf-8")
QStringList Mon = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sept","Oct","Nov","Dec"};
QStringList Week = { "Sun","Mon","Tue","Wed","Thur","Fri","Sat"};
template <class T>
T Range(T n,T tmin, T tmax)
{
    if (n < tmin)
        return tmin;
    else if (n > tmax)
        return tmax;
    else 
        return n;
}
KUSBLFTOASC::KUSBLFTOASC(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    connect(this, SIGNAL(sigInit(int)), this, SLOT(on_recInitState(int)));
    connect(this, SIGNAL(sigStatus(int)), this, SLOT(on_recConveterState(int)));
    ui.progressBar->setOrientation(Qt::Horizontal);// 水平方向
    QFile f(":/qrc/style.qss");
    if(f.open(QFile::ReadOnly))
    {
        this->setStyleSheet(f.readAll());
    }
    this->setWindowIcon(QIcon(":/qrc/logo.ico"));
    this->setWindowTitle("KUS BLF转ASC工具 V1.01.08");
    //connect(this, &KUSBLFTOASC::sigStatus, this, &KUSBLFTOASC::on_recConveterState);
}

KUSBLFTOASC::~KUSBLFTOASC()
{}

void KUSBLFTOASC::on_pushButton_sleBlf_clicked()
{
    m_sBlfName = QFileDialog::getOpenFileName(this, "选择BLF文件", QCoreApplication::applicationDirPath(), "BLF文件(*blf *BLF)");
    if (m_sBlfName.isEmpty())
        return;
    ui.lineEdit_blf->setText(m_sBlfName);
}

void KUSBLFTOASC::runConveter()
{
    //int read_test(LPCTSTR pFileName, LPDWORD pRead)
    //LPCSTR pFileName =("./b_2_15_81.blf");
    //LPCSTR pFileName = (m_sBlfName.toLocal8Bit().toStdString().c_str());
    QByteArray dd = m_sBlfName.toLocal8Bit();
    LPCSTR pFileName = dd.data();
    DWORD dwWritten;
    LONG64 sta = 0;
    QFile file(m_sAscName);
   

    DWORD *pRead=new DWORD();
    *pRead = 1;
    {
        HANDLE hFile;
        VBLObjectHeaderBase base;
        VBLCANMessage message;
        VBLCANMessage2_t message2;
        VBLCANErrorFrameExt erromessage;
        VBLEnvironmentVariable variable;
        VBLEthernetFrame ethframe;
        VBLAppText appText;
        VBLFileStatisticsEx statistics = { sizeof(statistics) };
        BOOL bSuccess;

        if (NULL == pRead)
        {
            emit sigInit(-88);
            return;
        }
        
        *pRead = 0;

        /* open file */
        hFile = BLCreateFile(pFileName, GENERIC_READ);

        if (INVALID_HANDLE_VALUE == hFile)
        {
            emit sigInit(-88);
            return ;
        }

        BLGetFileStatisticsEx(hFile, &statistics);
        int n = statistics.mObjectCount / 100;
        int mn = 0;
        emit sigInit(n);
        emit sigStatus(0);
        bSuccess = TRUE;

        bool isok = file.open(QIODevice::WriteOnly);
        if (!isok)
        {
            emit sigInit(-88);
            return;
        }
        QTextStream filestream(&file);
        /*QString data = "date " + Mon.at(statistics.mMeasurementStartTime.wDayOfWeek-1) + " "
            + Week.at(statistics.mMeasurementStartTime.wMonth-1)
            + " " + QString::number(statistics.mMeasurementStartTime.wDay)
            + " " + QString::number(statistics.mMeasurementStartTime.wHour)
            + ":" + QString::number(statistics.mMeasurementStartTime.wMinute)
            + ":" + QString::number(statistics.mMeasurementStartTime.wSecond)
            + "." + QString::number(statistics.mMeasurementStartTime.wMilliseconds)
            + "  " + QString::number(statistics.mMeasurementStartTime.wYear) + "\nbase hex  timestamps absolute\ninternal events logged\n// version N/A\n";*/

        QString data = "date " + Mon.at(Range(statistics.mMeasurementStartTime.wMonth-1,0,11)) + " "
            + QString::number(statistics.mMeasurementStartTime.wDay) + " "
            + Week.at(Range((int)statistics.mMeasurementStartTime.wDayOfWeek,0,6))
            + " " + QString::number(statistics.mMeasurementStartTime.wHour)
            + ":" + QString::number(statistics.mMeasurementStartTime.wMinute)
            + ":" + QString::number(statistics.mMeasurementStartTime.wSecond)
            + "." + QString::number(statistics.mMeasurementStartTime.wMilliseconds)
            + "  " + QString::number(statistics.mMeasurementStartTime.wYear) + "\nbase hex  timestamps absolute\ninternal events logged\n// version N/A\n";
        filestream << data;
        /* read base object header from file */
        while (RunState&&bSuccess && BLPeekObject(hFile, &base))
        {

            {
                message2.mHeader.mBase = base;
                bSuccess = BLReadObjectSecure(hFile, &message2.mHeader.mBase, sizeof(message2));
                /* free memory for the CAN message */
                if (!message2.mDLC)
                    continue;
                if (bSuccess) {
                    UINT64 id;
                    //CANoe的人在扩展帧ID上加了0x80000000的，所以要处理一下
                    if(message2.mID> 0x80000000)
                        id = message2.mID & 0x7FFFFFFF;
                    else
                        id = message2.mID;
                    /*printf("%ld 0x%x %2x %2x %2x %2x %2x %2x %2x %2x\n", message.mHeader.mObjectTimeStamp, (message.mID - 0x7FFFFFFF), message.mData[0],
                        message.mData[1], message.mData[2], message.mData[3], message.mData[4], message.mData[5], message.mData[6], message.mData[7]);*/
                    QString dir = CAN_MSG_DIR(message2.mFlags) == 0 ? "\tRx" : "\tTx";

                    QString str = QString("%1 %2  %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15")
                        .arg(QString::number(message2.mHeader.mObjectTimeStamp / 1000000000.0, 'f', 6))
                        .arg(QString::number(message2.mChannel))
                        .arg("0x" + QString::number(id, 16).toUpper())
                        .arg(dir)
                        .arg("d")
                        .arg(QString::number(message2.mDLC))
                        .arg(QString::number(message2.mData[0], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[1], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[2], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[3], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[4], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[5], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[6], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[7], 16).toUpper(), 2, QLatin1Char('0'))
                        /*.arg("Length = "+QString::number(message2.mFrameLength))
                        .arg("BitCount = "+QString::number(message2.mBitCount))*/
                        .arg("ID = " + QString::number(id) + "x") + "\n";
                    if (isFilter)
                    {
                        if (fifter.size() > 0)
                        {
                            for (int i = 0; i < fifter.size(); i++)
                            {
                                if (QString::number(id, 16).toUpper() == fifter.at(i).toUpper())
                                {
                                    filestream << str;
                                }
                            }
                        }
                    }
                    else
                    {
                        filestream << str;
                    }

                    BLFreeObject(hFile, &message2.mHeader.mBase);
                }
            }
            if (bSuccess)
            {
                *pRead += 1;
            }
            ++sta;
            if (sta % 100 == 0 || (sta / 100) >= n)
                emit sigStatus(++mn);
            if (!RunState)
            {
                filestream.flush();
                file.close();
                return;
            }
            continue;

            switch (base.mObjectType)
            {
            case BL_OBJ_TYPE_CAN_MESSAGE:
                /* read CAN message */
                message2.mHeader.mBase = base;
                bSuccess = BLReadObjectSecure(hFile, &message2.mHeader.mBase, sizeof(message2));
                /* free memory for the CAN message */
                if (bSuccess) {
                    QString id = QString::number(message2.mID - 0x7FFFFFFF, 16);
                    /*printf("%ld 0x%x %2x %2x %2x %2x %2x %2x %2x %2x\n", message.mHeader.mObjectTimeStamp, (message.mID - 0x7FFFFFFF), message.mData[0],
                        message.mData[1], message.mData[2], message.mData[3], message.mData[4], message.mData[5], message.mData[6], message.mData[7]);*/
                    QString dir = CAN_MSG_DIR(message2.mFlags) == 0 ? "\tRx" : "\tTx";
                    
                    QString str = QString("%1 %2  %3 %4 %5 %6 %7 %8 %9 %10 %11 %12 %13 %14 %15")
                        .arg(QString::number(message2.mHeader.mObjectTimeStamp/1000000000.0,'f',6))
                        .arg(QString::number(message2.mChannel))
                        .arg("0x" + QString::number((message2.mID - 0x7FFFFFFF)-1, 16).toUpper())
                        .arg(dir)
                        .arg("d")
                        .arg(QString::number(message2.mDLC))
                        .arg(QString::number(message2.mData[0],16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[1], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[2], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[3], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[4], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[5], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[6], 16).toUpper(), 2, QLatin1Char('0'))
                        .arg(QString::number(message2.mData[7], 16).toUpper(), 2, QLatin1Char('0'))
                        /*.arg("Length = "+QString::number(message2.mFrameLength))
                        .arg("BitCount = "+QString::number(message2.mBitCount))*/
                        .arg("ID = "+QString::number(message2.mID - 0x7FFFFFFF-1)+"x") + "\n";
                    if (isFilter)
                    {
                        if (fifter.size() > 0)
                        {
                            for (int i = 0; i < fifter.size(); i++)
                            {
                                if (id.toUpper() == fifter.at(i).toUpper())
                                {
                                    filestream << str;
                                }
                            }
                        }
                    }
                    else
                    {
                        filestream << str;
                    }
                   
                        BLFreeObject(hFile, &message2.mHeader.mBase);
                }
                break;
            case BL_OBJ_TYPE_ENV_INTEGER:
            case BL_OBJ_TYPE_ENV_DOUBLE:
            case BL_OBJ_TYPE_ENV_STRING:
            case BL_OBJ_TYPE_ENV_DATA:
                /* read environment variable */
                variable.mHeader.mBase = base;
                bSuccess = BLReadObjectSecure(hFile, &variable.mHeader.mBase, sizeof(variable));
                /* free memory for the environment variable */
                if (bSuccess) {
                    BLFreeObject(hFile, &variable.mHeader.mBase);
                }
                break;
            case BL_OBJ_TYPE_ETHERNET_FRAME:
                /* read ethernet frame */
                ethframe.mHeader.mBase = base;
                bSuccess = BLReadObjectSecure(hFile, &ethframe.mHeader.mBase, sizeof(ethframe));
                /* free memory for the frame */
                if (bSuccess) {
                    BLFreeObject(hFile, &ethframe.mHeader.mBase);
                }
                break;
            case BL_OBJ_TYPE_APP_TEXT:
                /* read text */
                appText.mHeader.mBase = base;
                bSuccess = BLReadObjectSecure(hFile, &appText.mHeader.mBase, sizeof(appText));
                if (NULL != appText.mText)
                {
                    printf("%s\n", appText.mText);
                }
                /* free memory for the text */
                if (bSuccess) {
                    BLFreeObject(hFile, &appText.mHeader.mBase);
                }
                break;
            case BL_OBJ_TYPE_UNKNOWN:
                break;
            case BL_OBJ_TYPE_CAN_ERROR:
            case BL_OBJ_TYPE_CAN_ERROR_EXT:
                erromessage.mHeader.mBase = base;
                break;
            default:
                /* skip all other objects */
                bSuccess = BLSkipObject(hFile, &base);
                break;
            }

            if (bSuccess)
            {
                *pRead += 1;
            }
            ++sta;
            if(sta%100==0 || (sta/100)>= n)
                emit sigStatus(++mn);
            if (!RunState)
            {
                filestream.flush();
                file.close();
                return;
            }
        }
        filestream.flush();
        file.close();
        emit sigStatus(n);
        emit sigInit(-99);
        /* close file */
        if (!BLCloseHandle(hFile))
        {
            return ;
        }

        //return bSuccess ? 0 : -1;
    }
}

void KUSBLFTOASC::on_pushButton_selAsc_clicked()
{
    m_sAscName = QFileDialog::getSaveFileName(this,"保存asc文件", QCoreApplication::applicationDirPath(), "ASC文件(*asc *asc)");
    if (m_sAscName.isEmpty())
        return;
    ui.lineEdit_asc->setText(m_sAscName);
}

void KUSBLFTOASC::on_pushButton_converter_clicked()
{
    if (ui.checkBox_filter->isChecked())
    {
        fifter = ui.lineEdit_filter->text().split(";");
        isFilter = true;
    }
    else
    {
        isFilter = false;
    }
    if (m_sBlfName.isEmpty())
    {
        QMessageBox::warning(this, "BLF文件为空", "BLF文件不能为空");
    }
    ui.pushButton_converter->setEnabled(false);
    QtConcurrent::run(this, &KUSBLFTOASC::runConveter);
}

void KUSBLFTOASC::on_recConveterState(int statue)
{
    ui.progressBar->setValue(statue);
}

void KUSBLFTOASC::on_recInitState(int max)
{
    if (max == -99)
    {
        QMessageBox::information(this, "完成", "Perfect! 转换完成");
        ui.progressBar->setValue(0);
        ui.pushButton_converter->setEnabled(true);
        return;
    }
    else if (max == -88)
    {
        QMessageBox::information(this, "未完成", "fail! 转换失败");
        ui.progressBar->setValue(0);
        ui.pushButton_converter->setEnabled(true);
        return;
    }
    else
    {
        ui.progressBar->setMaximum(max);
        ui.progressBar->setMinimum(0);
    }
   
}

void KUSBLFTOASC::closeEvent(QCloseEvent* event)
{
    RunState = false;
    Sleep(300);
    
}
