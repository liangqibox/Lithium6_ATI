#ifndef QISERIAL
#define QISERIAL


#include "windows.h"
#include "tchar.h"

class QiSerial
{
public:
    QiSerial(){
        return;
    }

    ~QiSerial(){
        Close();
    }

    int Open(){
        const TCHAR Port[5]= {'C','O','M','3'};
        hSerial = CreateFile(Port,GENERIC_READ | GENERIC_WRITE,0,0,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,0);
        if(hSerial==INVALID_HANDLE_VALUE){
            if(GetLastError()==ERROR_FILE_NOT_FOUND){
                //serial port does not exist. Inform user.
                return 1;
            }
            //some other error occurred. Inform user.
            return 2;
        }

        DCB dcbSerialParams;
        dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            //error getting state
            return 3;
        }
        dcbSerialParams.BaudRate=CBR_115200;
        dcbSerialParams.ByteSize=8;
        dcbSerialParams.StopBits=ONESTOPBIT;
        dcbSerialParams.Parity=NOPARITY;
        if(!SetCommState(hSerial, &dcbSerialParams)){
            //error setting serial port state
            return 3;
        }

        COMMTIMEOUTS timeouts;
        timeouts.ReadIntervalTimeout=50;
        timeouts.ReadTotalTimeoutConstant=50;
        timeouts.ReadTotalTimeoutMultiplier=10;
        timeouts.WriteTotalTimeoutConstant=50;
        timeouts.WriteTotalTimeoutMultiplier=10;
        if(!SetCommTimeouts(hSerial, &timeouts)){
            //error occureed. Inform user
            return 3;
        }
        return 0;
    }

    bool Close(){
        return CloseHandle(hSerial);
    }

    int ReadData(void *Buff, int n){
        DWORD dwBytesRead = 0;
        if(!ReadFile(hSerial, Buff, n, &dwBytesRead, NULL)){
            return 0;
        }
        return int(dwBytesRead);
    }

    int SendData(char *Buff, int n){
        DWORD dwBytesRead = 0;
        if(!WriteFile(hSerial, Buff, n, &dwBytesRead, NULL)){
            return 0;
        }
        return int(dwBytesRead);
    }

protected:
    HANDLE hSerial;

};


#endif // QISERIAL

