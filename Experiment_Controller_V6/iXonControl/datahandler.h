#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QList>
#include <stdlib.h>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QRegExp>

#include <QMutex>
#include <vector>
#include <stdio.h>

#include "ixoncamera.h"
#include "png++/png.hpp"


class DataHandler : public QObject
{
    Q_OBJECT

public:
    DataHandler();

public slots:
    void getLastAcqData(int xDP, int yDP, int numKin);
    void getNewImage(int xDP, int yDP);
    void saveDataPlainText(QVector<ushort> data, QString filename);
    void saveDataPNG16(QVector<ushort> data, QString filename);


    void saveImages(QString filename, bool all);
    void saveLastAcqData(int xDP, int yDP, int numKin);
    void saveVideoFramePNG(int xDP, int yDP, int fNum, bool save);

    inline bool savingInProgress() {return m_savingInProgress;}

    inline QString filename() {return m_filename;}
    inline QString filepath() {return m_filepath;}
    inline int saveMode() {return m_saveMode;}

    inline QString tempFilename() {return "_temp" + QString::number(rand() % 10000);}


    void setSaveMode(int mode);

    QString filetags();
    void updateFilename(QString newfile);
    void updateFilepath(QString newpath);

    QString processedFilename(QString filename);

    void setAcqParameters(int runNum, int scanNum);

    void increaseSeriesCounter();

    void deleteTempFiles();


    void saveAll();
    void clearAll();


private:
    bool m_savingInProgress;
    QList<QVector<ushort> > dataList;

    QString m_filename;
    QString m_filetags;
    QString m_filepath;

    int m_runNum;
    int m_scanNum;
    int m_seriesNum;

    int m_tagIncrement;
    int m_saveMode; //0... png, 1...plaintext, 2...both

signals:
    void dataAppended();
    void frameAppended();
    void frameSaved(QString filename);
    void fileWritten();
    void saveModeChanged(int mode);
    void filepathChanged();
};

#endif // DATAHANDLER_H
