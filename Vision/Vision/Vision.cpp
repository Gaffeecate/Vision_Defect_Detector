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

// 전역 변수, 배열의 시작 주소를 알리는 포인터. 여기에 각 픽셀값이 하나씩 할당되게 됨
// unsigned char는 0~255의 값을 저장할 수 있는 타입으로 각 픽셀의 밝기값을 저장하는데 활용됨
// unsigned는 C++에서 부호가 없다는 뜻. 
unsigned char* g_pImageData = nullptr;

// IP 주소 파싱
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
CameraHandle CreateCamera()
{
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

    if (MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList) != MV_OK)
    {
        printf("Enum Devices fail!\n");
        return nullptr;
    }

    if (stDeviceList.nDeviceNum == 0)
    {
        printf("No camera found!\n");
        return nullptr;
    }

    CameraHandle handle = nullptr;
    MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[0];

    if (MV_CC_CreateHandle(&handle, pDeviceInfo) != MV_OK)
    {
        printf("Create Handle fail!\n");
        return nullptr;
    }

    if (MV_CC_OpenDevice(handle, MV_ACCESS_Exclusive, 0) != MV_OK)
    {
        printf("Open Device fail!\n");
        MV_CC_DestroyHandle(handle);
        return nullptr;
    }

    printf("Camera opened successfully\n");
    return handle;
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

void FinalizeCamera() {
    MV_CC_Finalize();
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

    return frame(roi).clone(); 
}


Mat detectAndMarkDefect(const Mat& frame, int& outDefectCount) { // ROI가 적용된 프레임을 가져옴
    Mat result = frame.clone(); // 원본 보호위함
    cvtColor(result, result, COLOR_GRAY2BGR);

    // 대비 향상
    Mat enhancedFrame;
    equalizeHist(frame, enhancedFrame);

    // 노이즈 제거, 대비향상 이후 수행되어야 함.
    Mat denoised;
    GaussianBlur(enhancedFrame, denoised, Size(5, 5), 0);

    // 적응형 이진화
    Mat binaryMask;
    adaptiveThreshold(enhancedFrame, binaryMask, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 31, 5);

    // 추가적인 노이즈 제거 (모폴로지 연산)
    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    morphologyEx(binaryMask, binaryMask, MORPH_OPEN, kernel);
    //morphologyEx(binaryMask, binaryMask, MORPH_CLOSE, kernel);

    // 윤곽선 찾기
    vector<vector<Point>> contours; // 윤곽선을 저장할 벡터선언
    findContours(binaryMask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    outDefectCount = 0;
    double minDefectArea = 800;  // 최소 결함 면적

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
    namedWindow("Enhanced Frame", WINDOW_NORMAL);
    resizeWindow("Enhanced Frame", 800, 600);
    imshow("Enhanced Frame", enhancedFrame);

    namedWindow("Binary Mask", WINDOW_NORMAL);
    resizeWindow("Binary Mask", 800, 600);
    imshow("Binary Mask", binaryMask);

    return result;
}


int main()
{
    // 이미지 데이터 버퍼 할당, 이미지 처리할때 메모리 할당해놓는거다
    g_pImageData = new unsigned char[MAX_IMAGE_DATA_SIZE];
    if (g_pImageData == nullptr)
    {
        printf("Failed to allocate memory\n");
        return -1;
    }

    // 카메라 생성 및 초기화
    CameraHandle handle = CreateCamera();
    if (handle == nullptr)
    {
        printf("Failed to create camera handle\n");
        delete[] g_pImageData;
        return -1;
    }


    // 카메라 설정
    if (SetExposureAuto(handle, false) != MV_OK ||
        SetExposure(handle, 30000.0f) != MV_OK || 
        SetFramerate(handle, 5.9f) != MV_OK || 
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

    // ArduinoCommunicator 객체 생성
    ArduinoCommunicator arduino(L"\\\\.\\COM3"); // \\\ -> 윈도우 장치 직접접근 문법, 시리얼 포트 3번으로

    // 서보 모터 시작 명령 전송
    try {
        arduino.sendCommand("START\n");
    }
    catch (const exception& e) {
        printf("Failed to send start command: %s\n", e.what());
        CloseCamera(handle);
        delete[] g_pImageData;
        return -1;
    }

    // 메인 루프
    int frameCount = 0;
    auto lastTime = chrono::steady_clock::now();

    namedWindow("Camera Feed", WINDOW_NORMAL);
    resizeWindow("Camera Feed", 800, 600);

    while (true)
    {
        try
        {
            Mat frame = GetFrame(handle);
            if (!frame.empty())
            {

                // ROI 처리 및 확대
                Mat zoomedFrame = ProcessFrameWithROI(frame, frame.cols / 5, frame.rows / 5);  // ROI 크기를 조정


                // 결함 감지 및 표시
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
                    catch (const exception& e) {
                        printf("Failed to send move command: %s\n", e.what());
                    }
                }
            }
            else
            {
                cout << "Empty frame received!" << endl;
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

    StopGrabbing(handle);
    CloseCamera(handle);
    FinalizeCamera();
    delete[] g_pImageData;

    return 0;
}