#include "datahandler.h"

#define VIDEOTEMPFILENUM 3

DataHandler::DataHandler() : QObject(0)
{
    m_seriesNum = 1;
    m_savingInProgress = false;

    updateFilepath(QDir::toNativeSeparators(QDir::currentPath()));
    updateFilename("testdata");

}

void DataHandler::clearAll()
{
    dataList.clear();
    return;
}

void DataHandler::saveAll()
{
    saveImages(m_filepath + m_filename, true);
}

void DataHandler::getLastAcqData(int xDP, int yDP, int numKin) //any reason to use the 32bit version of GetAcqData?
{
    QVector<ushort> data;
    if(numKin < 1) numKin = 1; //quick
    unsigned long size  = xDP * yDP;
    data.resize(size*numKin + 2);

    data[0] = xDP;
    data[1] = yDP;
    int err_check = GetAcquiredData16(data.data() + 2, size * numKin);
    qDebug() << "sample out" << data[134] << "err" << err_check;

    if(err_check == DRV_SUCCESS)
    {
        if(numKin > 1) //quite slow, mb need to find another way
        {
            QVector<ushort> kinOut;
            for(int i = 0; i < numKin; i++)
            {
                kinOut = {xDP, yDP};
                std::vector<ushort> kinData(data.begin() + size*i + 2, data.begin() + size * (i+1) + 2);
                kinOut.append(QVector<ushort>::fromStdVector(kinData));
                dataList.push_back(kinOut); //mb slow?
            }
        }
        else dataList.push_back(data);
        emit dataAppended();
    }
    return;
}

void DataHandler::getNewImage(int xDP, int yDP)
{
    QVector<ushort> data;
    unsigned long size  = xDP * yDP;
    data.resize(size + 2);

    data[0] = xDP;
    data[1] = yDP;
    int err_check = GetMostRecentImage16(data.data() + 2, size);
    if(err_check == DRV_SUCCESS)
    {
        dataList.push_back(data);
        emit frameAppended();
    }
    qDebug() << "frame sample out" << data[134] << "err" << err_check;

    return;
}

void DataHandler::saveDataPlainText(QVector<ushort> data, QString filename)
{
    QMutex mutex;
    mutex.lock();
    QFileInfo info(filename);
    QString tempName = info.path() + QDir::separator() + tempFilename();

    if(data.isEmpty()) return;
    QFile outfile(tempName + ".dat");
    if(outfile.open(QFile::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&outfile); //Debugging
        int x = 1;
        int y = 1;
        ushort xDP = data[0];
        ushort yDP = data[1];
        for(auto i = data.begin() + 2; i != data.end(); i++)
        {
            out << x << " " << y << " " << *i << "\n\r";
            x++;
            if(x > xDP && y < yDP)
            {
                x = 1;
                y++;
            }
        }
    }
    outfile.close();

    if(QFile::rename(tempName + ".dat", filename + ".dat") == false)
    {
        QFile::remove(filename + ".dat");
        QFile::rename(tempName + ".dat", filename + ".dat");
    }

    mutex.unlock();
    emit fileWritten();
    return;
}

void DataHandler::saveDataPNG16(QVector<ushort> data, QString filename)
{
    if(data.isEmpty()) return;

    QMutex mutex;
    mutex.lock();
    QFileInfo info(filename);
    QString tempName = info.path() + QDir::separator() + tempFilename();


    ushort xDP = data[0];
    ushort yDP = data[1];
    png::image<png::gray_pixel_16> image(xDP, yDP);

    for(png::uint_32 y = 0; y < image.get_height(); y++)
    {
        for(png::uint_32 x = 0; x < image.get_width(); x++)
        {
            image[y][x] = png::gray_pixel_16(data[x + y*xDP + 2]);
        }
    }
    image.write((tempName + ".PNG").toStdString());

    if(QFile::rename(tempName + ".PNG", filename + ".PNG") == false)
    {
        QFile::remove(filename + ".PNG");
        QFile::rename(tempName + ".PNG", filename + ".PNG");
    }

    mutex.unlock();
    emit fileWritten();
    return;
}

void DataHandler::saveImages(QString filename, bool all) //improve this
{
    QString fTags;
    m_seriesNum = 1;

    while(!dataList.isEmpty())
    {
        fTags = filetags();
        if(m_saveMode == 0 || m_saveMode == 2) saveDataPNG16(dataList.first(), processedFilename(filename));
        if(m_saveMode == 1 || m_saveMode == 2) saveDataPlainText(dataList.first(), processedFilename(filename));
        dataList.pop_front();
        increaseSeriesCounter();
        if(!all) break;
    }
    return;
}

void DataHandler::saveLastAcqData(int xDP, int yDP, int numKin)
{
    m_savingInProgress = true;
    getLastAcqData(xDP, yDP, numKin);
    saveImages(m_filepath + m_filename, true);
    m_savingInProgress = false;
    return;
}

void DataHandler::saveVideoFramePNG(int xDP, int yDP, int fNum, bool save) //rewrite this
{
    m_savingInProgress = true;
    getNewImage(xDP, yDP);
    QString fname;

    if(!save)
    {
        fNum = fNum % VIDEOTEMPFILENUM;
        fname = m_filepath + "_tempVideoFrame_" + QString::number(fNum);
        saveDataPNG16(dataList.last(), fname);
        dataList.pop_back();
    }
    else //this should call saveImages, but cant since this renames the files
    {
        fname = processedFilename(m_filepath + m_filename + "_" + QString::number(fNum));
        if(m_saveMode == 0 || m_saveMode == 2) saveDataPNG16(dataList.last(), fname);
        if(m_saveMode == 1 || m_saveMode == 2) saveDataPlainText(dataList.last(), fname);
        dataList.pop_back();
        increaseSeriesCounter();
    }

    emit frameSaved(fname);
    m_savingInProgress = false;
    return;
}

void DataHandler::setSaveMode(int mode)  //0...png, 1...plaintext, 2...both
{
    if(mode < 0 || mode > 2) return;
    else m_saveMode = mode;
    emit saveModeChanged(m_saveMode);
    return;
}

QString DataHandler::filetags() //simple counter for now
{
    return "_ " + QString::number(m_tagIncrement++);
}


void DataHandler::updateFilename(QString newfile)
{
    if(newfile != "") m_filename = newfile;
    emit filepathChanged();
    return;
}

void DataHandler::updateFilepath(QString newpath)
{
    if(newpath == "") return;
    if(!newpath.endsWith(QDir::separator())) newpath.append(QDir::separator());
    m_filepath = QDir::toNativeSeparators(newpath);
    emit filepathChanged();
    return;
}

QString DataHandler::processedFilename(QString filename)
{
    if(!filename.isEmpty())
    {
        QDateTime dateTime = QDateTime::currentDateTime();
        QString yr = QString("%1").arg(dateTime.date().year(), 4, 10, QChar('0'));
        QString mnth = QString("%1").arg(dateTime.date().month(), 2, 10, QChar('0'));
        QString day = QString("%1").arg(dateTime.date().day(), 2, 10, QChar('0'));
        QString h = QString("%1").arg(dateTime.time().hour(), 2, 10, QChar('0'));
        QString m = QString("%1").arg(dateTime.time().minute(), 2, 10, QChar('0'));
        QString s = QString("%1").arg(dateTime.time().second(), 2, 10, QChar('0'));


        filename.replace("<YYYY>", yr, Qt::CaseSensitive).isEmpty();
        filename.replace("<YY>", yr.right(2), Qt::CaseSensitive);

        filename.replace("<MM>", mnth, Qt::CaseSensitive);
        filename.replace("<DD>", day, Qt::CaseSensitive);
        filename.replace("<hh>", h, Qt::CaseSensitive);
        filename.replace("<mm>", m, Qt::CaseSensitive);
        filename.replace("<ss>", s, Qt::CaseSensitive);

        filename.replace("<run>", QString::number(m_runNum), Qt::CaseSensitive);
        filename.replace("<scan>", QString::number(m_scanNum), Qt::CaseSensitive);
        filename.replace("<series>", QString::number(m_seriesNum), Qt::CaseSensitive);


        //CLEANUP FOR MISUSED <  >
        QRegExp rx("<*>");
        rx.setPatternSyntax(QRegExp::Wildcard);
        filename.replace(rx, "");
    }

    return filename;
}


void DataHandler::setAcqParameters(int runNum, int scanNum)
{
    m_seriesNum = 1;
    m_runNum = runNum;
    m_scanNum = scanNum;

    return;
}

void DataHandler::increaseSeriesCounter()
{
    m_seriesNum++;
    return;
}

void DataHandler::deleteTempFiles()
{
    QDir dataDir(m_filepath);
    QStringList filter;
    filter << "_temp*";
    dataDir.setNameFilters(filter);
    QStringList fileList = dataDir.entryList();
    for(auto i = fileList.begin(); i != fileList.end(); i++)
    {
        dataDir.remove(*i);
    }
    return;
}
