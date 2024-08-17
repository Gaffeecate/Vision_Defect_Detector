#pragma once

#include <windows.h>
#include <string>

class ArduinoCommunicator {
private:
    HANDLE hSerial;
    DCB dcbSerialParams;
    COMMTIMEOUTS timeouts;

public:
    ArduinoCommunicator(const std::wstring& portName);
    ~ArduinoCommunicator();

    void sendCommand(const std::string& command);

    // 복사 생성자와 대입 연산자를 명시적으로 삭제
    ArduinoCommunicator(const ArduinoCommunicator&) = delete;
    ArduinoCommunicator& operator=(const ArduinoCommunicator&) = delete;
};