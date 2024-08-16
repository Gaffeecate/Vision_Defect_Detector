#include <stdio.h>
#include <string>
#include <opencv2/opencv.hpp>
#include "MvCameraControl.h"
#include <windows.h>
#include <chrono>

using namespace std;
using namespace cv;

#define MAX_IMAGE_DATA_SIZE (5472 * 3648)

// 카메라 핸들 타입 정의
typedef void* CameraHandle;

// 전역 변수
unsigned char* g_pImageData = nullptr;

// 유틸리티 함수: IP 주소 파싱
bool ParseIPAddress(const string& ip_address, unsigned int& nIP)
{
    unsigned int nIP1, nIP2, nIP3, nIP4;
    if (sscanf_s(ip_address.c_str(), "%u.%u.%u.%u", &nIP1, &nIP2, &nIP3, &nIP4) == 4)
    {
        nIP = (nIP1 << 24) | (nIP2 << 16) | (nIP3 << 8) | nIP4;
        return true;
    }
    return false;
}

// 카메라 생성 및 초기화
CameraHandle CreateCamera(const string& cameraname, const string& ip_address)
{
    int nRet = MV_OK;
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    // GigE 카메라 열거
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList);
    if (MV_OK != nRet)
    {
        printf("Enum Devices fail! nRet [0x%x]\n", nRet);
        return nullptr;
    }

    printf("Found %d device(s)\n", stDeviceList.nDeviceNum);

    CameraHandle handle = nullptr;
    bool cameraFound = false;
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            string modelName((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName);
            printf("Device %d: Model: %s, IP: %d.%d.%d.%d\n", i, modelName.c_str(),
                (pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp >> 24) & 0xFF,
                (pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp >> 16) & 0xFF,
                (pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp >> 8) & 0xFF,
                pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xFF);

            if (modelName == cameraname)
            {
                cameraFound = true;
                nRet = MV_CC_CreateHandle(&handle, pDeviceInfo);
                if (MV_OK != nRet)
                {
                    printf("Create Handle fail! nRet [0x%x]\n", nRet);
                    return nullptr;
                }

                // IP 주소 설정
                if (!ip_address.empty())
                {
                    unsigned int nIP;
                    if (ParseIPAddress(ip_address, nIP))
                    {
                        nRet = MV_GIGE_ForceIpEx(handle, nIP, 0xFFFFFF00, 0x0101A8C0);
                        if (MV_OK != nRet)
                        {
                            printf("Set IP fail! nRet [0x%x]\n", nRet);
                            MV_CC_DestroyHandle(handle);
                            return nullptr;
                        }
                    }
                    else
                    {
                        printf("Invalid IP address format\n");
                        MV_CC_DestroyHandle(handle);
                        return nullptr;
                    }
                }

                // 장치 열기
                nRet = MV_CC_OpenDevice(handle, MV_ACCESS_Exclusive, 0);
                if (MV_OK != nRet)
                {
                    printf("Open Device fail! nRet [0x%x]\n", nRet);
                    // 추가적인 오류 정보 출력
                    if (nRet == 0x80000206)
                    {
                        printf("Error: Access Denied. The camera might be in use by another application.\n");
                    }
                    else if (nRet == 0x80000203)
                    {
                        printf("Error: Invalid Handle. The camera handle might be corrupted.\n");
                    }
                    else if (nRet == 0x80000201)
                    {
                        printf("Error: Camera Offline. Check the physical connection and power supply.\n");
                    }
                    MV_CC_DestroyHandle(handle);
                    return nullptr;
                }

                printf("Camera opened successfully\n");
                return handle;
            }
        }
    }

    if (!cameraFound)
    {
        printf("No matching camera found!\n");
    }

    return nullptr;
}

// 카메라 설정 함수들
int SetExposureAuto(CameraHandle handle, bool isauto)
{
    return MV_CC_SetEnumValue(handle, "ExposureAuto", isauto ? 2 : 0);
}

int SetExposure(CameraHandle handle, float exposure)
{
    return MV_CC_SetFloatValue(handle, "ExposureTime", exposure);
}

int SetFramerate(CameraHandle handle, float framerate)
{
    return MV_CC_SetFloatValue(handle, "AcquisitionFrameRate", framerate);
}

int StartGrabbing(CameraHandle handle)
{
    return MV_CC_StartGrabbing(handle);
}

int StopGrabbing(CameraHandle handle)
{
    return MV_CC_StopGrabbing(handle);
}

void CloseCamera(CameraHandle handle)
{
    if (handle != nullptr)
    {
        MV_CC_CloseDevice(handle);
        MV_CC_DestroyHandle(handle);
    }
}

// 이미지 획득 함수
Mat GetFrame(CameraHandle handle)
{
    MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
    memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));

    int nRet = MV_CC_GetOneFrameTimeout(handle, g_pImageData, MAX_IMAGE_DATA_SIZE, &stImageInfo, 1000);
    if (nRet == MV_OK)
    {
        Mat frame(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, g_pImageData);
        return frame.clone();
    }
    else
    {
        printf("Failed to get frame, error code: [0x%x]\n", nRet);
        if (nRet == 0x8000000A)
        {
            printf("Timeout error occurred. Check camera connection and settings.\n");
        }
    }
    return Mat();
}


// ROI 설정 및 확대 함수
Mat ProcessFrameWithROI(const Mat& frame, int roiWidth, int roiHeight)
{
    int centerX = frame.cols * 7 / 20;
    int centerY = frame.rows / 2;
    Rect roi(centerX - roiWidth / 2, centerY - roiHeight / 2, roiWidth, roiHeight);

    Mat zoomedFrame;
    resize(frame(roi), zoomedFrame, Size(frame.cols, frame.rows));
    return zoomedFrame;
}



int main()
{
    // 이미지 데이터 버퍼 할당
    g_pImageData = new unsigned char[MAX_IMAGE_DATA_SIZE];
    if (g_pImageData == nullptr)
    {
        printf("Failed to allocate memory\n");
        return -1;
    }

    // 카메라 생성 및 초기화
    CameraHandle handle = CreateCamera("MV-CS200-10GM", "");
    if (handle == nullptr)
    {
        printf("Failed to create camera handle\n");
        delete[] g_pImageData;
        return -1;
    }

    // 카메라 설정
    if (SetExposureAuto(handle, false) != MV_OK ||
        SetExposure(handle, 30000.0f) != MV_OK ||
        SetFramerate(handle, 5.0f) != MV_OK ||
        MV_CC_SetEnumValue(handle, "PixelFormat", PixelType_Gvsp_Mono8) != MV_OK)
    {
        printf("Failed to set camera parameters\n");
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    // 스트림 설정 추가
    if (MV_CC_SetEnumValue(handle, "TriggerMode", 0) != MV_OK)
    {
        printf("Failed to set trigger mode\n");
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    // 이미지 획득 시작
    if (StartGrabbing(handle) != MV_OK)
    {
        printf("Failed to start grabbing\n");
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    // 시리얼 통신 설정
    HANDLE hSerial;
    DCB dcbSerialParams = { 0 };
    COMMTIMEOUTS timeouts = { 0 };

    LPCWSTR portName = L"\\\\.\\COM3";  // Arduino가 연결된 COM 포트로 설정. 내껀 COM3
    hSerial = CreateFileW(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        printf("Error opening serial port\n");
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        printf("Error getting serial port state\n");
        CloseHandle(hSerial);
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    dcbSerialParams.BaudRate = CBR_9600;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        printf("Error setting serial port state\n");
        CloseHandle(hSerial);
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        printf("Error setting timeouts\n");
        CloseHandle(hSerial);
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    // 메인 루프
    int currentAngle = 0;
    auto lastTime = std::chrono::steady_clock::now();

    namedWindow("Camera Feed", WINDOW_NORMAL);
    resizeWindow("Camera Feed", 800, 600);

    while (true)
    {
        try
        {
            Mat frame = GetFrame(handle);
            if (!frame.empty())
            {
                imshow("Camera Feed", frame);

                // ROI 처리 및 확대
                Mat processedFrame = ProcessFrameWithROI(frame, 400, 400); // 예: 200x200 크기
                imshow("Camera Feed", processedFrame);

                // 1초마다 각도 업데이트 및 전송
                auto currentTime = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime).count() >= 1)
                {
                    currentAngle += 10;
                    if (currentAngle > 180) currentAngle = 0;

                    char angleStr[10];
                    sprintf_s(angleStr, "%d\n", currentAngle);
                    DWORD bytesWritten;
                    WriteFile(hSerial, angleStr, strlen(angleStr), &bytesWritten, NULL);

                    lastTime = currentTime;
                    printf("Sent angle: %d\n", currentAngle);
                }
            }
        }
        catch (const Exception& e)
        {
            printf("OpenCV Exception: %s\n", e.what());
            break;
        }
        catch (const exception& e)
        {
            printf("Standard Exception: %s\n", e.what());
            break;
        }
        catch (...)
        {
            printf("Unknown Exception Occurred\n");
            break;
        }

        char c = (char)waitKey(1);
        if (c == 27)  // ESC key
            break;
    }

    // 정리
    StopGrabbing(handle);
    CloseCamera(handle);
    delete[] g_pImageData;
    CloseHandle(hSerial);

    return 0;
}