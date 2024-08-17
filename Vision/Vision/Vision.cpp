#include <stdio.h>
#include <string>
#include <opencv2/opencv.hpp>
#include "MvCameraControl.h"
#include "Includes/ArduinoCommunicator.h"
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

int SetGain(CameraHandle handle, float gain)
{
    return MV_CC_SetFloatValue(handle, "Gain", gain);
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

    return frame(roi).clone();  // ROI만 적용하고 크기 조정은 하지 않음
}


Mat detectAndMarkDefect(const Mat& frame, int& outDefectCount) {
    Mat result = frame.clone();
    cvtColor(result, result, COLOR_GRAY2BGR);

    // 대비 향상
    Mat enhancedFrame;
    equalizeHist(frame, enhancedFrame);

    // 적응형 이진화
    Mat binaryMask;
    adaptiveThreshold(enhancedFrame, binaryMask, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 31, 5);

    // 노이즈 제거
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(binaryMask, binaryMask, MORPH_OPEN, kernel);
    morphologyEx(binaryMask, binaryMask, MORPH_CLOSE, kernel);

    // 윤곽선 찾기
    vector<vector<Point>> contours;
    findContours(binaryMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    outDefectCount = 0;
    double minDefectArea = 800;  // 최소 결함 면적 (필요에 따라 조정)

    for (const auto& contour : contours) {
        double area = contourArea(contour);
        if (area > minDefectArea) {
            Rect boundingRect = cv::boundingRect(contour);
            rectangle(result, boundingRect, Scalar(0, 0, 255), 2);  // 빨간색 사각형
            outDefectCount++;
        }
    }

    // 결함 개수 표시 (크기와 위치 조정, 외곽선 추가)
    string text = outDefectCount > 0 ? "Defect Detected: " + to_string(outDefectCount) : "No Defects";
    int fontFace = FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.5;
    int thickness = 2;
    int baseline = 0;
    Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
    Point textOrg((result.cols - textSize.width) / 2, textSize.height + 10);

    // 텍스트 외곽선 그리기 (검정색)
    putText(result, text, textOrg, fontFace, fontScale, Scalar(0, 0, 0), thickness * 3);
    // 텍스트 내부 그리기 (흰색)
    putText(result, text, textOrg, fontFace, fontScale, Scalar(255, 255, 255), thickness);

    // 디버깅을 위한 중간 결과 표시
    imshow("Enhanced Frame", enhancedFrame);
    imshow("Binary Mask", binaryMask);

    return result;
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

    // ArduinoCommunicator 객체 생성
    ArduinoCommunicator arduino(L"\\\\.\\COM3");

    // 카메라 설정
    if (SetExposureAuto(handle, false) != MV_OK ||
        SetExposure(handle, 30000.0f) != MV_OK ||
        SetFramerate(handle, 5.9f) != MV_OK ||
        SetGain(handle, 0.0f) != MV_OK ||
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

    // 서보 모터 시작 명령 전송
    try {
        arduino.sendCommand("START\n");
    }
    catch (const std::exception& e) {
        printf("Failed to send start command: %s\n", e.what());
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    // 메인 루프
    int frameCount = 0;
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

                // ROI 처리 및 확대 (순서 변경)
                Mat zoomedFrame = ProcessFrameWithROI(frame, frame.cols / 5, frame.rows / 5);  // ROI 크기를 조정


                // 결함 감지 및 표시 (수정된 함수 사용)
                int defectCount;
                Mat processedFrame = detectAndMarkDefect(zoomedFrame, defectCount);


                imshow("Camera Feed", processedFrame);


                // 매 15프레임마다 서보 모터 이동 명령 전송
                frameCount++;
                if (frameCount % 15 == 0)
                {
                    try {
                        arduino.sendCommand("MOVE\n");
                        printf("Sent move command to servo\n");
                    }
                    catch (const std::exception& e) {
                        printf("Failed to send move command: %s\n", e.what());
                    }
                }
            }
            else
            {
                cout << "Empty frame received!" << endl;
            }
        }
        catch (const cv::Exception& e)
        {
            printf("OpenCV Exception: %s\n", e.what());
            break;
        }
        catch (const std::exception& e)
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

    StopGrabbing(handle);
    CloseCamera(handle);
    delete[] g_pImageData;

    return 0;
}