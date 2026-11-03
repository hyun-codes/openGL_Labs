# 학교 PC OpenGL 시작하기

Windows / Visual Studio 2022 또는 2026의 MSVC x64 / VS Code 기준이다.
경로를 D드라이브 등에 고정하지 않으므로 폴더 전체를 다른 위치로 옮겨도 된다.

## Visual Studio에서 ▶ / F5로 실행하기

VS Code 없이 Visual Studio만 있어도 된다.

1. 아래 시작용 ZIP의 최신 버전을 받아 압축을 푼다.
2. **InitOpenGL.sln을 더블클릭한다.** 폴더 열기나 main.cpp만 열기로 들어가지 않는다.
3. Visual Studio 위쪽에서 **Debug / x64**를 확인한다.
4. **Ctrl+F5**로 실행하거나, 중단점 디버깅은 **F5 / 로컬 Windows 디버거 ▶**를 누른다.
5. 소스를 수정하고 Ctrl+S로 저장한 뒤 다시 Ctrl+F5를 누르면 빌드하고 실행한다.

시작 프로젝트가 선택되지 않았다면 솔루션 탐색기에서 InitOpenGL 프로젝트를 우클릭하고 **시작 프로젝트로 설정**한다.
프로젝트를 열 때 C++ 도구 설치 안내가 나오면 C++를 사용한 데스크톱 개발을 설치한다.
헤더·라이브러리 경로와 DLL 복사는 프로젝트에 설정되어 있다. build.bat을 별도로 실행할 필요가 없다.
위쪽에 **구성 없음**이 보이면 솔루션 대신 폴더나 파일만 연 상태이므로 파일 → 열기 → 프로젝트/솔루션에서 InitOpenGL.sln을 연다.

## 1. 가장 쉬운 방법: 준비된 ZIP 하나 받기

**[OpenGL 학교용 시작 폴더 다운로드](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip)**

1. ZIP을 다운로드한다.
2. 마우스 오른쪽 → 모두 압축 풀기를 누른다. ZIP 안에서 바로 실행하지 않는다.
3. 압축을 푼 곳의 `InitOpenGL` 폴더를 원하는 위치로 옮긴다. 예: `C:\OpenGLStudy`.
4. Visual Studio에서는 InitOpenGL.sln을 연다. VS Code에서는 **파일 → 폴더 열기**로 그 폴더를 연다.
5. 폴더 바로 아래에 `main.cpp`, `build.bat`, `externals`가 함께 있는지 확인한다.

이 ZIP에는 초기 코드, Visual Studio 솔루션·프로젝트, 빌드·실행 파일, VS Code 설정, GLFW·GLEW·GLM 라이브러리가 모두 들어 있다.
폴더 이름은 바꿔도 된다. 컴파일러는 ZIP에 포함하지 않으므로 3번 설치 확인은 필요하다.

## 2. 직접 폴더를 구성하거나 코드만 복사하는 방법

**[라이브러리만 다운로드: externals.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/externals.zip)**

새 폴더를 만들고, 이 GitHub 폴더의 `main.cpp`, `build.bat`, `run.bat`, `.vscode`를 복사한다.
Visual Studio를 사용한다면 `InitOpenGL.sln`과 `InitOpenGL.vcxproj`도 함께 복사한다.
externals.zip을 압축 해제하고 그 안의 **externals 폴더 전체**를 새 폴더 안에 넣는다.

```text
C:\OpenGLStudy\                  ← VS Code에서 여는 폴더
├─ InitOpenGL.sln                 ← Visual Studio에서는 이 파일을 연다
├─ InitOpenGL.vcxproj             ← Visual Studio 빌드·라이브러리 설정
├─ main.cpp                      ← 초기화·메인 루프·배경 그리기
├─ build.bat                     ← 컴파일·라이브러리 연결·DLL 복사 자동 처리
├─ run.bat                       ← 프로그램 실행
├─ .vscode\                      ← 없어도 터미널 빌드는 가능
│  ├─ tasks.json                 ← Ctrl+Shift+B 빌드 설정
│  ├─ c_cpp_properties.json      ← 헤더 경로·코드 자동완성 설정
│  └─ extensions.json            ← C/C++ 확장 추천
└─ externals\
   ├─ glew\
   │  ├─ include\GL\glew.h
   │  ├─ lib\Release\x64\glew32.lib
   │  └─ bin\Release\x64\glew32.dll
   ├─ glfw\
   │  ├─ include\GLFW\glfw3.h
   │  ├─ lib-vc2022\             ← glfw3dll.lib, glfw3.dll
   │  └─ lib-vc2026\             ← glfw3dll.lib, glfw3.dll
   └─ glm\
      └─ glm\glm.hpp
```

**externals\externals\glew처럼 폴더가 두 번 중첩되면 안 된다.**
친구 저장소의 `config/opengl.props`는 Visual Studio 프로젝트용이다.
이 VS Code 시작 폴더에서는 build.bat이 헤더·링커·DLL 경로를 설정하므로 props 파일이 필요 없다.

## 3. 학교 PC에서 한 번만 확인할 설치 항목

1. **Visual Studio Installer**를 연다.
2. Visual Studio 2022/2026 또는 Build Tools에서 **수정**을 선택한다.
3. **C++를 사용한 데스크톱 개발**을 선택한다. MSVC x64/x86 도구와 Windows SDK를 설치한다.
4. VS Code를 설치한다.
5. VS Code 확장에서 **C/C++ — Microsoft**를 설치한다.

학교 PC에 Visual Studio와 C++ 개발 도구가 이미 있으면 다시 설치하지 않아도 된다.
VS Code와 C/C++ 확장만으로는 컴파일러가 설치되지 않는다.
설치 권한이 필요하면 학교 PC 관리자에게 C++ 개발 도구 설치를 요청해야 한다.
MinGW용 빌드 설정이 아니라 MSVC용이다.

## 4. 빌드하기

VS Code에서 main.cpp가 있는 폴더를 연 다음 **터미널 → 새 터미널**을 누른다.
PowerShell에서 아래 명령을 입력한다.

```powershell
.\build.bat
```

`Build OK`가 나오면 성공이다. MSVC 개발 환경은 build.bat이 자동으로 찾는다.
일반 PowerShell에서 실행해도 되며 별도로 x64 Native Tools 명령창을 열 필요가 없다.
VS 2022면 lib-vc2022, VS 2026이면 lib-vc2026을 선택한다.

또는 **Ctrl+Shift+B**로 빌드할 수 있다.
상단의 Code Runner 실행 버튼 대신 이 빌드 방법을 사용한다.

빌드 결과는 다음과 같다.

```text
C:\OpenGLStudy\bin\
├─ OpenGLStudy.exe
├─ glew32.dll
├─ glfw3.dll
└─ main.obj
```

## 5. 실행하기

```powershell
.\run.bat
```

800×600 크기의 푸른 배경 창이 뜨면 GLFW·GLEW·GLM 연결이 완료된 것이다.
ESC로 종료한다. VS Code의 **터미널 → 작업 실행 → OpenGL: run**도 사용할 수 있다.
프로그램을 종료한 뒤 코드를 수정하고, 다시 build.bat → run.bat 순서로 실행한다.
DLL은 빌드할 때 자동으로 실행 파일 옆에 복사되므로 직접 옮길 필요가 없다.

## 6. 초기 코드 흐름

```text
main
  → InitOpenGL
      → GLFW 초기화
      → OpenGL 3.3 Core 창 생성
      → 현재 컨텍스트 설정
      → GLEW 초기화
      → 뷰포트·깊이 테스트 설정
  → 반복
      → DrawScene
      → 버퍼 교체
      → 이벤트 처리
  → Cleanup
```

이후 정점·셰이더·VAO·VBO 코드를 추가한다. 이 초기 코드는 아직 도형을 그리지 않는다.
OpenGL 3.3 Core이므로 glRectf나 glBegin 대신 셰이더와 VAO·VBO를 사용한다.
build.bat은 현재 main.cpp 하나를 컴파일한다. 별도 cpp 파일을 추가하면 cl 명령에도 그 파일을 추가한다.
셰이더·models를 추가할 경우 run.bat은 프로젝트 폴더를 작업 경로로 사용한다.

## 7. 문제가 생겼을 때

| 메시지·증상 | 해결 |
| --- | --- |
| externals is missing | 폴더 중첩 여부와 glew/glfw/glm 위치 확인 |
| MSVC C++ x64 tools were not found | Visual Studio Installer에서 C++ 데스크톱 개발 설치 |
| Windows SDK 헤더·라이브러리가 없음 | 같은 설치 관리자에서 Windows SDK 설치 |
| LNK1104로 OpenGLStudy.exe를 열 수 없음 | 실행 중인 창을 종료하고 다시 빌드 |
| DLL이 없다는 오류 | build.bat을 다시 실행하고 bin 안의 DLL 두 개 확인 |
| 헤더에 빨간 밑줄만 표시됨 | C/C++ 확장 설치 후 C/C++: Select IntelliSense Configuration에서 설치된 MSVC x64 선택 |
| OpenGL 3.3 윈도우 생성 실패 | 학교 PC의 GPU·그래픽 드라이버가 OpenGL 3.3을 지원하는지 확인 |

## 라이브러리 출처

라이브러리 ZIP은 로컬에 받아 둔 [snooy-dev/opengl-labs](https://github.com/snooy-dev/opengl-labs)의 externals 폴더를 그대로 묶었다.
GLFW·GLEW·GLM의 기존 라이선스 파일을 포함한다. 배포되는 바이너리는 출처 폴더의 파일을 재포장한 것이며 여기서 새로 컴파일한 것이 아니다.
