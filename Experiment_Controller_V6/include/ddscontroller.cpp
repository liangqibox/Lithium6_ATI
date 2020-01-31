#include "ddscontroller.h"
#include <QSerialPortInfo>

#include <QDebug>

#define FUN_SET_FREQUENCY       'f'
#define FUN_SET_AMPLITUDE       'a'
#define FUN_SET_PHASE           'p'
#define FUN_TRIGGER             't'
#define FUN_RESET               'R'
#define FUN_LINEAR_SWEEP        'l'
#define FUN_LINEAR_SWEEP_FULL   'L'
#define FUN_SHORT_PULSE         'P'
#define FUN_SWEEP_RESET         'r'
#define FUN_WAIT_FOR_TRIGGER    'w'

/*
 * Commands:
 *
 * !PROG<uint8:len><data><crc16>
 * !RUN
 * !STOP
 * !TRIG
 *
 * Responses:
 * !OK
 * !ERR<uint8:code>
 */

DDSCycle::DDSCycle() {
    buffer.open(QBuffer::WriteOnly);
}

QByteArray DDSCycle::getCycleData() {
    return buffer.buffer();
}

void DDSCycle::reset() {
    buffer.putChar(FUN_RESET);
}

void DDSCycle::trigger() {
    buffer.putChar(FUN_TRIGGER);
}

void DDSCycle::wait_for_trigger() {
    buffer.putChar(FUN_WAIT_FOR_TRIGGER);
}

void DDSCycle::singleTone(float freq) {
    buffer.putChar(FUN_SET_FREQUENCY);
    buffer.write((char*)&freq, sizeof(float));
}

void DDSCycle::setAmplitude(float ampl) {
    buffer.putChar(FUN_SET_AMPLITUDE);
    buffer.write((char*)&ampl, sizeof(float));
}

void DDSCycle::setPhase(float phase) {
    buffer.putChar(FUN_SET_PHASE);
    buffer.write((char*)&phase, sizeof(float));
}

void DDSCycle::linearSweep(float fromFreq, float toFreq, float duration) {
    buffer.putChar(FUN_LINEAR_SWEEP);
    buffer.write((char*)&fromFreq, sizeof(float));
    buffer.write((char*)&toFreq, sizeof(float));
    buffer.write((char*)&duration, sizeof(float));
}

void DDSCycle::linearSweep(float fromFreq, float toFreq, float duration, bool nodwell, float timestep) {
    buffer.putChar(FUN_LINEAR_SWEEP_FULL);
    buffer.write((char*)&fromFreq, sizeof(float));
    buffer.write((char*)&toFreq, sizeof(float));
    buffer.write((char*)&duration, sizeof(float));
    buffer.write((char*)&timestep, sizeof(float));
    if(nodwell) {
        buffer.putChar('1');
    } else {
        buffer.putChar('0');
    }
}

void DDSCycle::shortPulse(float freq, float duration) {
    buffer.putChar(FUN_SHORT_PULSE);
    buffer.write((char*)&freq, sizeof(float));
    buffer.write((char*)&duration, sizeof(float));
}

void DDSCycle::resetSweep() {
    buffer.putChar(FUN_SWEEP_RESET);
}



void DDSController::onDataReady() {
    while(serial.bytesAvailable() > 0) {
        char c;
        serial.getChar(&c);

        //qInfo() << "Data received: " << c;

        if(c == '\n') {
            emit statusReceived(msg);
            msg.clear();
        } else {
            msg.append(c);
        }
    }
}

DDSController::DDSController(QString port, QObject *parent) : QObject(parent),serial(port)
{
    // TODO: Handle error

    //qDebug() << port;
    //serial.setPort(QSerialPortInfo(port));
    serial.setBaudRate(QSerialPort::Baud9600);
    if(serial.open(QIODevice::ReadWrite)){
        this->stop_cycle();
        opened = true;
    }


    else opened = false;

    /*QString buf;

    // wait until the microcontroller signals it is ready by sending "start"
    while(true) {
        if(serial.bytesAvailable() == 0)
            serial.waitForReadyRead(500);
            //serial.waitForReadyRead();

        char c;
        serial.getChar(&c);
        buf.append(c);

        if(buf.contains("ready")) {
            break;
        }
    }


    connect(&serial, SIGNAL(readyRead()), this, SLOT(onDataReady()));*/
}

DDSController::~DDSController() {
    serial.close();
}

bool DDSController::isOpen(){
    return opened;
}

void DDSController::Close(){
    serial.close();
}

QList<QString> DDSController::availablePorts() {
    // TODO: Check each port if device is an Arduino Due

    QList<QSerialPortInfo> portInfos = QSerialPortInfo::availablePorts();
    QList<QString> result;

    for(QList<QSerialPortInfo>::iterator it = portInfos.begin(); it != portInfos.end(); ++it) {
        if(it->vendorIdentifier() ==  0x2a03 && it->productIdentifier() == 0x3d)
            result.append(it->portName());
    }

    return result;
}

/*
 * CRC16 implementation from Wikipedia: Computation of cyclic redundancy checks
 */
uint16_t crc(QByteArray data) {
    uint16_t rem = 0;
    for(int i = 0; i < data.size(); ++i) {
        rem  ^=  data[i];
        for(int j = 0; j < 8; ++j) {   // Assuming 8 bits per byte
            if(rem & 0x0001) {   // if rightmost (most significant) bit is set
                rem  = (rem >> 1) ^ 0x8408;
            } else {
                rem  = rem >> 1;
            }
        }
    }
    return rem;
 }

uint8_t readResponse(QSerialPort& serial) {
    QString buf = "";

    serial.waitForBytesWritten(2000);

    while(true) {
        // wait until data is available
        if(serial.bytesAvailable() == 0) {
            if(!serial.waitForReadyRead(2000)) {
                return DDS_ERROR_TIMEOUT;
            }
        }

        char c;
        serial.getChar(&c);

        if(buf.length() == 0) {
            // every response starts with !
            if(c == '!') {
                buf.append(c);
            } else {
                continue;
            }
        } else if (buf == "!ERR") {
            return c;
        } else {
            buf.append(c);
        }

        if(buf == "!OK") {
            return 0;
        }
    }
}

uint8_t DDSController::program_cycle(DDSCycle &cycle) {
    QByteArray cycleData = cycle.getCycleData();
    uint8_t len = cycleData.size();
    uint16_t crc16 = crc(cycleData);

    serial.write("!PROG");
    serial.write((char*)&len, sizeof(len));
    serial.write(cycleData);
    serial.write((char*)&crc16, sizeof(crc16));

    //qDebug() << serial.error();
    return readResponse(serial);
}

uint8_t DDSController::send_trigger() {
    serial.write("!TRIG");

    return readResponse(serial);
}

uint8_t DDSController::start_cycle() {
    serial.write("!RUN");

    return readResponse(serial);
}

uint8_t DDSController::stop_cycle() {
    serial.write("!STOP");

    return readResponse(serial);
}
