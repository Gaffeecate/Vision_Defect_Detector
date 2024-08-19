# Vision_Defect_Detector

## 프로젝트 개요

### 산업용 머신비전을 활용한 결함 감지 시스템
   
서보모터가 돌아가면서 결함이 새겨진 피사체를 회전시키고, 비전에 나타난 피사체의 결함을 인식하여 결함의 개수를 카운트 하는 프로그램입니다.
이를 위해 OpenCV 라이브러리에서 제공하는 이미지 처리함수, Camera API를 활용했습니다.

Language: C++    
IDE/Editor: Visual Studio 2022, VS Code(Arduino)    
Library: OpenCV 4.6.0     
Sub-Software: HIKROBOT MVS 64Bit V4.3.2 build20240520 (SDK: V4.4.0.4)    


## 프로젝트 플로우

![image](https://github.com/user-attachments/assets/7bc1e6c6-782b-43e1-9257-a8f2dd8aabcb)


## 프로젝트 배경/과정

처음 기획의도는 지금 가지고 있는 웹캠으로 구현하는 것 이었습니다.
산업용 머신비전의 카메라의 경우 가격도 비싸고, 후술하겠지만 연결방식도 일반적인 USB 단자를 가진 웹캠에 비해서 복잡한 방식으로 구현해야 합니다.
하지만 설비 S/W를 다루게 된다면 익숙해져야 할 부분이고, 비전과 컴퓨터를 연결만 할 수 있다면 그 이후 구현에선 많은 자료가 있으므로 해볼만한 프로젝트라고 생각하였습니다.

개발기간은 대략 40일 정도 소요되었습니다. 가장 큰 부분은 준비물 선정기간이었고 특히 산업용 렌즈의 경우 해외배송 건이라 2주가 넘는 시간이 소요되었습니다. 그 2주의 기간 동안 웹캠을 활용하여
개발을 하였고(Machine_Vision_Pre 레포지토리 참조) OpenCV, Visual Studio 개발환경에 익숙해지려고 노력했습니다. 또한 일찍 배송 된 아두이노 세트를 가지고 모션 쪽 구현을 미리 진행하였습니다.

렌즈가 도착하고, 렌즈를 가지고 있던 머신비전 카메라에 부착하고 본격적인 개발을 시작하였습니다. Camera 제조사에서 제공한 SDK를 설치한 후에 작동을 확인하고 API를 통해 연결을 구현하였습니다.
렌즈부착 -> Visual Studio 코드로 연결확인까지 대략 5일이 소요되었습니다. 

이후 아두이노 보드와 통신하는 부분과 Defect 감지 부분을 구현했습니다. 해당부분에서 7~8일 정도 소요되었습니다.


## H/W 선정

| No.    | 이름        | 모델명        | 가격      | 비고      |
|--------|-------------|---------------|----------|-----------|
| 1 | Machine Vision Camera      |     HIKROBOT-MV-CS200-10GM    | 89,000원 | Used, GigE |
| 2 | Camera Lens       | FUJINON HF16HA-1B 고정 초점 산업용 렌즈       | 72,280원 | Used, 16mm |
| 3 | Servo Motor, Board, Connector        | 아두이노 우노 R3 초보자 키트    | 24,200원 | - |
| 4 | POE Injector      | TP Link TL-POE150S        | 15,900원 | - |
| 5 | Lan Cable(Cat 6)      | LD-GPST/BU20-D        | 5,000원 | - |
| 6 | 기타      | 조명(보유 중인 LED 조명), 클램프        | - | - |
| - | Total      | -       | 206,380원 | - |

비전과 카메라 파트의 재료선정은 가격을 최우선으로 생각하였습니다. 그리고 카메라의 경우 프레임 그레버 타입과 GigE 타입 중에서
가지고 있는 PC가 노트북이고 가격역시 프레임 그레버 카드가 고가에, 호환성 여부를 확신할 수 없었으므로, 길게 고민하지 않고 랜선으로 연결되는 GigE 타입의 
HIKROBOT-MV-CS200-10GM을 선정하였습니다. 구매처는 [리사이클 메카](http://www.rpom.co.kr/)입니다.

렌즈의 경우 위 카메라와 호환되는 C-마운트 렌즈 중에서 대략 30~50cm 정도의 거리에서 촬영가능한 렌즈로 선정하였습니다. 
산업용 렌즈에서 주로 쓰이는 초점거리 12mm와 16mm 카메라 렌즈를 고려하였고, 계획된 피사체의 결함크기가 크지 않은것을 고려하여 16mm 렌즈를 선정하였습니다. 
해당 모델의 선정은 가격을 우선시하여 선정하였고, 지마켓에서 중고물품으로 구매하였습니다. 

한가지 주의해야할 점은 구매 전 반드시 Format size를 확인해야합니다. Format 사이즈란 렌즈에 최적화된 'Camera sensor size'를 가리킵니다.
해당모델의 포멧사이즈는 2/3인치이고, 카메라 센서사이즈는 1인치입니다. 그 말은 렌즈의 [이미지 서클](https://blog.naver.com/PostView.naver?blogId=sng82&logNo=50124841250)이 카메라 대각선 직경보다 작다는것을 의미하고 Full size 이미지에서
[비네팅/Vignetting(가장자리가 어두워짐)](https://ko.wikipedia.org/wiki/%EB%B9%84%EB%84%A4%ED%8C%85) 현상이 발생할 수 있습니다. 저의 경우에도 풀 사이즈 이미지에선 비네팅 현상이 있었습니다.

나머지 다른 물품들은 인터넷 및 오프라인 매장에서 쉽게 구매하였습니다. LAN Cable은 단거리 연결에서 CAT 버전은 크게 의미가 없으므로 시중에서 가장 저렴한 제품을 고르시면 됩니다.


## H/W 연결

Vision: 노트북 혹은 가정용 PC에 GigE 타입의 머신비전 카메라를 연결할 때 중요한 것은 랜선을 통해 파워를 공급하는 것 입니다.
이때 필요한 요소가 파워를 공급해주는 POE 인젝터입니다. 중간에 카메라로 공급되는 파워를 주는 역할입니다. 연결은 아래 도식대로 하시면 됩니다.

![image](https://github.com/user-attachments/assets/10ac93d7-47b2-4244-b6bc-4efee66b6be3)

Arduino: 서보모터와 보드간 연결을 해야합니다. 회로연결은 저도 처음이었는데 복잡하지 않아서 키트 제조사나 Arduino Uno R3 보드 + Servo motor를 검색하시면 
금방 연결회로도를 알 수 있습니다. 서보모터는 크게 전원(VCC) 핀, 접지(GND), 제어신호핀이 있습니다. 전원핀은 5V 핀, 접지는 GND 핀, 제어신호핀은 아두이노의 PWM 출력핀 중 하나에 
꽂아놓습니다. Digital PWM 핀은 입력 신호를 받아 서보가 특정각도로 회전하게 합니다. 

![image](https://github.com/user-attachments/assets/e9cb8010-8d79-4849-9ff8-42a33f65909b)

추가로 아두이노 보드와 PC를 연결할때 중간에 꽂는 포트가 달라져선 안됩니다. 처음 연결할때 포트를 설정하게 되는데, 이 포트에 해당하는 USB 단자에 꽂아야 작동하게 됩니다. 

이렇게 연결하고 나면 하드웨어 구성이 완료됩니다.

## 설치 및 실행방법

### Vision

우선 카메라 연결이 정상적인지부터 확인해야합니다. 이를 위해 HIKROBOT MVS를 설치해줍니다.
설치가 끝날때 동안 카메라 IP를 설정해줍니다. 
1. ```제어판\네트워크 및 인터넷\네트워크 및 공유 센터```로 들어가서
2. 이더넷(RealTek PCle ~~)라고 되어있는 아이콘 클릭
3. 인터넷 프로토콜 버전4 (TCP/IPv4)에 들어가서 속성 버튼 클릭
4. 자동으로 IP 주소 받기가 되어있는지 확인합니다.

고정 아이피를 사용하는 법도 있지만 해당 경우엔 카메라의 Default IP를 알고 있어야 합니다. 저의 경우엔 중고라서 아이피를 찾지못해 자동 아이피를 설정하였습니다.
아이피를 확인하고, MVS에 들어갑니다. 그럼 해당 화면처럼 나오게 되는데 랜선이 연결되어 있을 경우 이더넷[] 대괄호에 아이피가 나오게 됩니다. 

![image](https://github.com/user-attachments/assets/24a95486-2d88-44eb-8b58-2dd4f07c57b7)

클릭하시고 카메라가 나오는지 확인해주세요. 나오지 않는 경우 하드웨어 연결을 확인해주셔야 합니다. 
잘 나오는지 확인한 이후 Visual Studio 상에서 각자 카메라 하드웨어에 맞게 설정을 해주셔야합니다.

**참고로 HIKROBOT Vision Camera를 사용한다면 MVS를 설치했을때 `C:\Program Files (x86)\MVS\Development\Documentations`에 API 문서가 함께 제공됩니다. 개발할때 활용도가 높으니 참조하시면 됩니다.**

`CameraHandle handle = CreateCamera("MV-CS200-10GM", "");`
해당 부분에서 본인의 카메라 모델로 변경하고

```C++
    // 카메라 설정
    if (SetExposureAuto(handle, false) != MV_OK ||
        SetExposure(handle, 30000.0f) != MV_OK ||
        SetFramerate(handle, 5.9f) != MV_OK ||
        SetGain(handle, 0.0f) != MV_OK ||
        MV_CC_SetEnumValue(handle, "PixelFormat", PixelType_Gvsp_Mono8) != MV_OK)
```

해당 영역에서 카메라 스펙에 맞는 값들을 넣어주세요. 노출값, 프레임을 설정해야합니다. 기본값은 카메라의 상세스펙에 나와있습니다.

다음 비주얼 스튜디오 프로젝트 속성에서     
C++ -> 일반 -> 추가 포함 디렉터리: `$(OPENCV_DIR)\include;C:\Program Files (x86)\MVS\Development\Includes`   
링커 -> 일반 -> 추가 라이브러리 디렉터리: `C:\Program Files (x86)\MVS\Development\Libraries\win64;$(OPENCV_DIR)\x64\vc15\lib`    
링커 -> 입력 -> 추가 종속성: `opencv_world460.lib;mvCameraControl.lib;%(AdditionalDependencies)`    

이렇게 입력해줍니다. 당연히 본인의 openCV 버전이나 Debug/Release 구성에 따라 달라져야합니다.

위 과정을 마치면 이제 Vision Setting은 완료가 되었습니다.



### Arduino

VS Code를 사용하였으므로 아두이노 IDE 만을 사용한 과정보다 더 복잡합니다. VS Code를 사용한 의도는 실무와 조금이라도 더 비슷한 환경을 구성하려는 목적이었습니다만
너무 복잡하고 크게 필요하지 않았던 과정이므로 코드를 클론을 하시고 그냥 아두이노 IDE를 바로 사용하는 것을 권장합니다.

#### VS code 설정법

1. Arduino IDE를 설치합니다.
2. 해당 코드를 VS Code 사용하여 실행합니다.
3. 아두이노 확장을 설치해주세요
4. ```Ctrl+Shift+P```를 누르고 ```Preferences: Open Settings (JSON)``` 을 클릭합니다.
5. 아래 템플릿을 ```c_cpp_properties.json```에 붙여넣습니다.
 ``` JSON
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/",
                "C:/Users/Account Name/Documents/Arduino/libraries/",
                "C:/Users/Account Name/AppData/Local/Arduino15/hardware/avr/1.8.6/cores/arduino/",
                "C:/Users/Account Name/AppData/Local/Arduino15/hardware/avr/1.8.6/variants/"
            ],
            "defines": [
                "_DEBUG",
                "UNICODE",
                "_UNICODE",
                "USBCON"
            ],
            "cStandard": "c11",
            "cppStandard": "c++11",
            "intelliSenseMode": "gcc-x64",
            "compilerPath": "C:/Users/Account Name/AppData/Local/Arduino15/avr-gcc/7.3.0-atmel3.6.1-arduino7/bin/avr-gcc.exe"
        }
    ],
    "version": 4
}
```
5. 위 템플릿에서 includePath와 Compiler Path를 각자 맞는 경로로 바꿔주시고
6. arduino.json에는
``` JSON
{
    "board": "arduino:avr:uno",
    "port": "COM3",
    "sketch": "Servo.ino"
}
```
해당 json을 붙여넣습니다. 해당 내용도 본인에게 맞는 보드와 파일명으로 바꿔주시면 됩니다.    
7. ```Ctrl+Shift+P```를 누르고 ```Arduino: Select Board``` 을 클릭합니다. 목록이 나타나면 본인의 아두이노 보드를 선택해줍니다.    
8. 다시 ```Ctrl+Shift+P```를 누르고 ```Arduino: Select Serial Port``` 을 클릭합니다. 본인의 포트가 나오면 선택해줍니다. 이건 보드가 PC에 꽂혀있는 상태여야 합니다.    
9. 이제 Servo.ino 파일을 누르고 우측 상단의 Arduino Verfy -> Arduino Upload를 눌러줍니다.     
10. vscode 터미널에 arduino done 이라는 메세지가 나오게 되면 코드가 아두이노 보드에 정상적으로 업로드 된것입니다.      

```Tree
C:.
│   .gitignore
│   Servo.ino
│
└───.vscode
        arduino.json
        c_cpp_properties.json
```

VS Code 상에서 위와같은 트리구조를 갖게 됩니다. 


## 핵심로직

### 카메라 연결

```C++
CameraHandle CreateCamera(const string& cameraname, const string& ip_address)
{
    MV_CC_DEVICE_INFO_LIST stDeviceList; // 연결된 카메라 장치 정보들을 담는 구조체
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST)); // memset 함수를 사용하여 stDeviceList 구조체으 모든 바이트를 0으로 초기화

    // GigE 카메라 열거
    if (MV_CC_EnumDevices(MV_GIGE_DEVICE, &stDeviceList) != MV_OK) // 연결된 카메라를 전부 불러와라
        return nullptr;

    CameraHandle handle = nullptr;
    for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
    {
        MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE) // MV_CC_DEVICE_INFO에 카메라 정보를 담고 GIGE 프로토콜 장치인지 확인
        {
            string modelName((char*)pDeviceInfo->SpecialInfo.stGigEInfo.chModelName); // 열거된 카메라의 모델명이
            if (modelName == cameraname) // 개발자가 입력한 카메라와 일치하냐
            {
                // 카메라 핸들 생성
                if (MV_CC_CreateHandle(&handle, pDeviceInfo) != MV_OK) // 그렇다면 핸들 생성
                    return nullptr;

                // IP 주소 설정
                if (!ip_address.empty())
                {
                    unsigned int nIP;
                    if (ParseIPAddress(ip_address, nIP))
                    {
                        if (MV_GIGE_ForceIpEx(handle, nIP, 0xFFFFFF00, 0x0101A8C0) != MV_OK)
                        {
                            MV_CC_DestroyHandle(handle);
                            return nullptr;
                        }
                    }
                }

                // 카메라 장치 열기
                if (MV_CC_OpenDevice(handle, MV_ACCESS_Exclusive, 0) != MV_OK)
                {
                    MV_CC_DestroyHandle(handle);
                    return nullptr;
                }

                return handle;
            }
        }
    }

    return nullptr;
}

```
웹캠의 경우, `VideoCapture cap(0);` 만으로도, 간단하게 카메라를 열 수 있지만 산업용 카메라의 경우 복잡한 과정을 거치면서 연결됩니다.
카메라의 주소값, 연결된 카메라들의 장치정보들을 불러와야 합니다. 

우선 `MV_CC_EnumDevices` 함수에서 연결 가능한 카메라 정보를 가져오고, 찾는 카메라의 프로토콜을 지정합니다. 이후 발견된 정보들을 구조체에 담습니다.
이후 `MV_CC_CreateHandle` 함수를 통해 카메라 핸들을 생성합니다. 카메라 핸들은 구조체에서 가져온 카메라정보를 기반으로 카메라와 연결을 수행합니다. 이미지 획득을 위해서 필수적입니다. 
최종적으로 핸들 즉 채널이 생성되면 카메라를 연결합니다.   

처음엔 복잡한 것처럼 보여도 아래 설명서의 플로우를 따라가다 보면 흐름은 눈에 보이는것 같습니다. 주황색으로 표시된 부분은 옵션 절차입니다.
IP 주소를 정확하게 알고있어 연결하고자 하는 카메라 정보를 가져올수 있는 경우엔 MV_CC_EnumDevices를 삭제 할수도 있습니다.

![image](https://github.com/user-attachments/assets/8b9d4047-8bdd-4168-b9d7-c05ad25dd8fb)


### 이미지 처리

```C++
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
    double minDefectArea = 800;  // 최소 결함 면적
}
  
```
1. 대비향상으로 결함에 있는 밝기차이를 증가시킵니다
2. 이진화(Binaryzation)을 통해서 이미지를 흑과 백 두가지로 나눕니다. 이미지처리를 단순화하여 속도를 내기 위함입니다.
3. 노이즈 제거를 통해 이진화 과정에서 생기는 노이즈들을 제거합니다. 
4. 최종적으로 윤곽선을 추출하여 결함의 경계를 식별합니다.


## 실제 구동장면

![convert](https://github.com/user-attachments/assets/b978d7fa-3d9b-43ee-a5a7-589492136e87)

## 참고자료

1. [Github](https://github.com/parkeh-dev/Control_Hikvision_Camera)
2. [OpenCV4로 배우는 컴퓨터 비전과 머신러닝](https://thebook.io/006939/0488/)
3. HIKROBOT API Document (Machine Vision Camera SDK Developer Guide Windows (C) V4.4.0)
