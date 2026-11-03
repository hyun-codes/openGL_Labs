# 학교 PC에서 Visual Studio로 OpenGL 실습 시작하기

**이 문서는 Visual Studio만 설치된 학교 PC 기준이다. VS Code는 필요 없다.**

목표: `C:\Opengl_codingTest` 폴더 하나를 만들고, 준비된 파일을 넣은 다음 Visual Studio의 **Ctrl+F5 / F5 / ▶**로 빌드하고 실행한다.

아래 **1번부터 순서대로** 따라간다. 준비 확인 → 다운로드 → 폴더 생성·압축 해제 → 솔루션 열기 → 실행 → 코드 수정 순서다.
처음 진행할 때는 전체 시작 ZIP 하나만 사용하면 된다.

## 1. Visual Studio의 C++ 개발 도구 확인

Visual Studio 프로그램만 설치되어 있고 C++ 컴파일러는 빠져 있을 수 있다.
학교 PC에서 C++ 프로젝트를 이미 실행한 적이 있다면 먼저 2번으로 진행해도 된다.

설치가 필요하다는 메시지가 나올 경우:

1. Windows 시작 메뉴에서 **Visual Studio Installer**를 검색해 실행한다.
2. 설치된 Visual Studio **2022 또는 2026** 옆의 **수정**을 누른다.
3. **워크로드** 화면에서 **C++를 사용한 데스크톱 개발**을 선택한다.
4. MSVC x64/x86 빌드 도구와 Windows SDK가 포함되어 있는지 확인한다.
5. **수정/설치**를 눌러 설치가 끝날 때까지 기다린다.

학교 PC에서 설치 권한이 없으면 담당자에게 해당 C++ 개발 도구 설치를 요청한다.
추가 OpenGL 설치 프로그램이나 VS Code 확장을 설치할 필요는 없다.

## 2. GitHub에서 시작용 ZIP 다운로드

1. [내 GitHub 저장소](https://github.com/hyun-codes/openGL_Labs)를 연다.
2. **InitOpenGL** 폴더를 누르고 이 README를 연다.
3. 아래 링크를 눌러 ZIP을 다운로드한다.

**[학교용 전체 ZIP 다운로드 — OpenGL-school-starter.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip)**

**처음 시작할 때는 이 전체 ZIP 하나만 받으면 된다.**
초기 코드, Visual Studio 프로젝트, GLFW·GLEW·GLM이 모두 들어 있다.
`shaders/vertex.glsl`, `shaders/fragment.glsl`과 셰이더 읽기·컴파일·연결 코드, 카메라 구조체, model/view/projection 전달까지 포함되어 있다. 셰이더 폴더를 직접 만들 필요 없다.
라이브러리 전용 externals.zip은 별도로 받을 필요 없다.

## 3. C드라이브에 실습 폴더 만들고 압축 풀기

1. 파일 탐색기에서 **내 PC → 로컬 디스크(C:)**를 연다.
2. 새 폴더를 만들고 이름을 **Opengl_codingTest**로 지정한다.
3. 다운로드한 `OpenGL-school-starter.zip`을 찾아 마우스 오른쪽 클릭 → **모두 압축 풀기**를 누른다.
4. 압축 해제 위치를 `C:\Opengl_codingTest`로 지정하고 압축을 푼다.
5. `C:\Opengl_codingTest` 안으로 들어가 **InitOpenGL** 폴더를 연다.

**ZIP을 더블클릭해서 압축파일 내부에서 코드를 여는 것이 아니라, 반드시 압축을 풀어야 한다.**

압축 프로그램이 ZIP 이름으로 폴더를 하나 더 만들었다면, 그 안의 **InitOpenGL 폴더 전체**를 `C:\Opengl_codingTest` 바로 아래로 옮긴다.
최종적으로 다음 경로가 되어야 한다.

```text
C:\Opengl_codingTest\InitOpenGL\InitOpenGL.sln
```

## 4. 폴더 구조 확인

```text
C:\
└─ Opengl_codingTest\
   └─ InitOpenGL\                  ← 실제 코드·프로젝트가 있는 폴더
      ├─ InitOpenGL.sln             ← Visual Studio에서 여는 파일
      ├─ InitOpenGL.vcxproj         ← 라이브러리·컴파일·실행 설정
      ├─ main.cpp                   ← 내가 수정하는 C++ 코드
      ├─ PracticeStarterGL.h        ← 기존 실습의 공통 셰이더·VAO·VBO 함수
      ├─ CubeVertices.h             ← 15·16번의 정육면체 정점 36개
      ├─ CUBE_USAGE.md              ← 정점 사용·버퍼 초기화·그리기 예제
      ├─ shaders\                   ← 이미 포함되어 있음
      │  ├─ vertex.glsl             ← model/view/projection 변환
      │  └─ fragment.glsl           ← 정점 색상 × objectColor
      ├─ README.md
      ├─ build.bat                  ← 명령창 빌드용. Visual Studio에서는 사용하지 않아도 됨
      ├─ run.bat                    ← 명령창 실행용. Visual Studio에서는 사용하지 않아도 됨
      ├─ .vscode\                   ← VS Code용 추가 설정. 지금은 사용하지 않음
      └─ externals\                 ← 라이브러리 전체
         ├─ glew\
         │  ├─ include\GL\glew.h
         │  ├─ lib\Release\x64\glew32.lib
         │  └─ bin\Release\x64\glew32.dll
         ├─ glfw\
         │  ├─ include\GLFW\glfw3.h
         │  ├─ lib-vc2022\           ← glfw3dll.lib, glfw3.dll
         │  └─ lib-vc2026\           ← glfw3dll.lib, glfw3.dll
         └─ glm\
            └─ glm\glm.hpp
```

확인할 핵심은 **InitOpenGL.sln / InitOpenGL.vcxproj / main.cpp / externals가 같은 위치에 있어야 한다**는 것이다.
`externals\externals\glew`처럼 externals가 중복되면 안 된다.

## 5. Visual Studio에서 올바르게 열기

### 탐색기에서 여는 방법

1. `C:\Opengl_codingTest\InitOpenGL` 폴더를 연다.
2. **InitOpenGL.sln**을 더블클릭한다.
3. 사용할 프로그램을 물으면 **Visual Studio 2022 또는 2026**을 선택한다.

### Visual Studio가 이미 열려 있는 경우

1. 상단 메뉴에서 **파일 → 열기 → 프로젝트/솔루션**을 누른다.
2. `C:\Opengl_codingTest\InitOpenGL`로 이동한다.
3. **InitOpenGL.sln**을 선택하고 **열기**를 누른다.
4. 저장할지 물으면 수정한 코드를 저장한다.

**이 단계에서는 「폴더 열기」나 main.cpp만 열기를 선택하지 않는다.**
이미 준비된 프로젝트를 사용하므로 **새 프로젝트 만들기, 빈 프로젝트 만들기, 데스크톱 마법사도 필요 없다.**

정상적으로 열렸다면:

- 오른쪽 **솔루션 탐색기**에 솔루션 **InitOpenGL**과 그 아래 프로젝트 **InitOpenGL**이 보인다.
- 프로젝트를 펼치면 main.cpp가 보인다. 파일 표시 방식에 따라 소스 파일 아래에 있을 수도 있다.
- 상단에 **Debug**, **x64**, **로컬 Windows 디버거 ▶**가 보인다.
- main.cpp 탭이 「기타 파일」로만 표시되고 상단에 「구성 없음」이 보인다면 5번을 다시 확인한다.

솔루션 탐색기가 안 보이면 **보기 → 솔루션 탐색기**를 누른다. 단축키는 **Ctrl+Alt+L**이다.

## 6. Visual Studio에서 실행하기

1. 상단 구성은 **Debug**, 플랫폼은 **x64**로 둔다.
2. 오른쪽 솔루션 탐색기에서 **main.cpp**를 더블클릭한다.
3. **Ctrl+Shift+S**로 모두 저장한다.
4. **Ctrl+F5**를 누른다. 디버깅 없이 빌드하고 실행하는 단축키다.
5. 빌드할지 묻는 창이 나오면 **예**를 누른다.
6. **800×600의 푸른 배경 창**이 뜨는지 확인한다.
7. **ESC** 또는 창 오른쪽 위 **X**로 종료한다.

중단점을 사용해 디버깅하려면 **F5** 또는 위쪽의 **로컬 Windows 디버거 ▶**를 누른다.
상단의 「시작 항목 선택」이 계속 보이면 프로젝트가 정상적으로 열렸는지 먼저 확인한다.
프로젝트는 열렸지만 시작 프로젝트가 지정되지 않았다면 솔루션 탐색기의 **InitOpenGL 프로젝트**를 우클릭 → **시작 프로젝트로 설정**한다.

**Visual Studio로 실행할 때 build.bat과 run.bat을 따로 실행할 필요는 없다.**
헤더 경로, 링커 라이브러리, 실행할 프로그램, DLL 복사가 프로젝트에 설정되어 있다.
학교 PC마다 라이브러리 경로를 다시 입력하거나 DLL을 직접 복사할 필요 없다.

빌드가 끝나면 다음 위치에 실행 파일이 생긴다.

```text
C:\Opengl_codingTest\InitOpenGL\bin\VisualStudio\Debug\
├─ OpenGLStudy.exe
├─ glew32.dll
├─ glfw3.dll
└─ shaders\                       ← 빌드할 때 자동 복사
```

## 7. 코드 수정하고 다시 실행하는 순서

```text
실행 중인 OpenGL 창 종료
    → Visual Studio에서 main.cpp 수정
    → Ctrl+S로 저장
    → Ctrl+F5로 빌드·실행
```

처음에는 main.cpp의 **DrawScene** 안에서 배경색만 바꿔 확인하면 된다.

```cpp
glm::vec4 backgroundColor(0.2f, 0.3f, 0.5f, 1.0f);
```

위 코드를 아래처럼 바꾸고 저장한 후 Ctrl+F5로 실행한다.

```cpp
glm::vec4 backgroundColor(1.0f, 1.0f, 1.0f, 1.0f);
```

배경이 흰색으로 바뀌면 수정 → 빌드 → 실행 과정까지 확인된 것이다.

## 8. 실습 코드를 어디에 추가할지

정육면체가 필요하면 **[CubeVertices.h 사용 예제](CUBE_USAGE.md)**를 참고한다.
헤더를 include하고 정점을 VBO에 올리면 로봇·시소 등의 부품으로 재사용할 수 있다.

### 기존 19번 코드를 그대로 옮겨 실행할 때

1. 최신 전체 ZIP을 압축 해제하고 InitOpenGL.sln을 연다.
2. 기존 Practice19/main.cpp의 내용을 초기 main.cpp 전체와 교체한다. 두 코드의 내용을 합치지 않는다.
3. 복사한 코드의 첫 줄만 다음처럼 바꾼다.

```cpp
// 기존 실습 폴더 구조에서 사용한 경로
#include "../PracticeStarterGL.h"

// 이 시작 폴더에서 사용할 경로
#include "PracticeStarterGL.h"
```

위 두 줄을 모두 넣는 것이 아니라 기존 첫 줄을 새 줄로 교체한다.
PracticeStarterGL.h는 ZIP의 main.cpp 옆에 이미 들어 있다. 별도로 다운로드하거나 상위 폴더에 놓을 필요 없다.
포함된 shaders도 19번과 호환된다. 19번은 OBJ 모델 파일이 필요 없다.
Ctrl+S로 저장한 뒤 Debug / x64에서 Ctrl+F5로 실행한다.

### 초기 코드에서 직접 구현할 때

초기 코드의 흐름은 다음과 같다.

```text
main
  → InitOpenGL: GLFW·윈도우·컨텍스트·GLEW·뷰포트 초기화
      → GLSL 파일 읽기 → 셰이더 컴파일 → 프로그램 연결
  → 반복: DrawScene → 버퍼 교체 → 이벤트 처리
  → Cleanup: 윈도우와 GLFW 정리
```

- 현재 초기 코드에는 도형을 그리는 부분이 없다.
- 셰이더는 이미 포함·연결되어 있다. 이후 정점·VAO·VBO 초기화와 그리기를 추가하면 된다.
- 초기화 함수는 OpenGL 컨텍스트와 GLEW 초기화가 성공한 뒤, 메인 반복문 전에 호출한다.
- 도형을 그리는 호출은 DrawScene에 넣는다.
- OpenGL 3.3 Core 설정이므로 glRectf/glBegin 대신 셰이더와 VAO·VBO 방식을 사용한다.
- 별도 cpp 파일을 만들 때는 프로젝트 우클릭 → **추가 → 새 항목 → C++ 파일**로 추가한다. 탐색기에 파일을 복사하기만 하면 프로젝트에 자동으로 포함되지 않을 수 있다.
- 솔루션 탐색기에서 vertex.glsl과 fragment.glsl을 열어 수정할 수 있다. 실제 파일은 shaders 폴더에 있다. 실행 작업 경로는 InitOpenGL.vcxproj가 있는 폴더다. 셰이더는 실행 폴더에도 빌드 시 자동 복사된다.

### 이미 준비되어 있는 셰이더와 카메라

`ReadShaderFile → CompileShader → CreateShaderProgram`이 GLSL을 읽고 컴파일·연결한다.
InitOpenGL에서 셰이더 프로그램을 한 번 만들고, DrawScene에서 glUseProgram으로 사용한다.
Cleanup은 프로그램을 삭제한다. 파일 읽기·컴파일·연결 실패는 콘솔에 출력된다.

vertex.glsl에는 다음 변환이 들어 있다. 18·19번과 같은 방식이다.

```glsl
gl_Position = projectionTransform * viewTransform * modelTransform * vec4(vPosition, 1.0);
```

| 값 | C++에서 준비하는 내용 |
| --- | --- |
| modelTransform | 도형의 이동·회전·크기. 기본값은 단위행렬 |
| viewTransform | Camera 구조체의 position/target/up으로 glm::lookAt 계산 |
| projectionTransform | 시야각·창 비율·near/far로 glm::perspective 계산 |
| objectColor | 정점 색상에 곱하는 색. 기본 흰색이므로 정점 색상이 유지됨 |

Camera의 기본 위치는 (0,0,5), 바라보는 곳은 원점이다. 창 크기가 바뀌면 투영 비율도 갱신된다.
카메라를 움직이는 계산은 C++에서 하고 그 결과인 view 행렬을 셰이더에 전달한다.
현재 시작 환경에는 카메라 조작 키를 추가하지 않았다. camera.position 또는 camera.target을 바꾸면 다음 프레임에 반영된다.
이 셰이더의 VAO 속성 0번은 위치 vec3, 1번은 색상 vec3이다.
색상 버퍼 없이 위치만 쓰려면 1번 속성을 비활성화하고 glVertexAttrib3f(1,1,1,1)로 기본 흰색을 지정할 수 있다.

## 9. 문제가 생겼을 때

| 상황 | 확인할 내용 |
| --- | --- |
| 구성 없음 / 시작 항목 선택 | 파일 → 열기 → 프로젝트/솔루션 → InitOpenGL.sln을 연다. main.cpp만 열거나 폴더만 연 상태가 아닌지 확인한다. |
| InitOpenGL.sln이 없음 | 예전 ZIP일 수 있다. 문서 맨 아래 부록의 프로젝트 파일 ZIP 적용 절차를 따른다. |
| 시작 프로젝트를 선택하라는 메시지 | 솔루션 탐색기에서 InitOpenGL 프로젝트 우클릭 → 시작 프로젝트로 설정 |
| C++ 빌드 도구가 없다는 메시지 | 1번의 Visual Studio Installer에서 C++를 사용한 데스크톱 개발 설치 |
| Windows SDK를 찾을 수 없음 | Visual Studio Installer에서 Windows SDK 설치 후 솔루션을 다시 연다. |
| glew.h / glfw3.h / glm.hpp를 찾을 수 없음 | externals와 main.cpp가 같은 폴더 아래인지, externals가 중복되지 않았는지 4번 구조와 비교한다. |
| glew32.lib / glfw3dll.lib를 찾을 수 없음 | x64 설정인지, externals의 lib·lib-vc2022·lib-vc2026 폴더가 있는지 확인한다. |
| LNK1104로 OpenGLStudy.exe를 열 수 없음 | 이전에 실행한 OpenGL 창을 종료한 후 다시 빌드한다. |
| DLL이 없다는 메시지 | 빌드가 성공했는지 확인하고, bin\VisualStudio\Debug 안에 glew32.dll과 glfw3.dll이 있는지 확인한다. |
| 셰이더 파일을 열 수 없다는 메시지 | main.cpp 옆 shaders 폴더와 안의 glsl 두 파일을 확인한다. 프로젝트 속성 → 디버깅 → 작업 디렉터리가 프로젝트 폴더인지 확인한다. |
| 수정한 내용이 실행에 반영되지 않음 | 솔루션 탐색기의 main.cpp를 수정했는지, Ctrl+S로 저장했는지 확인하고 Ctrl+Shift+B로 빌드 후 Ctrl+F5로 실행한다. |
| 빌드 오류 후 이전 버전을 실행할지 물음 | 아니요를 누르고 보기 → 출력의 빌드 메시지에서 첫 오류를 확인한다. |
| OpenGL 3.3 윈도우 생성 실패 | GPU와 그래픽 드라이버의 OpenGL 3.3 지원을 확인한다. |

## 10. 다른 실습용 폴더를 새로 만들 때

동작을 확인한 InitOpenGL 폴더 전체를 복사하고 폴더 이름만 바꿔도 된다.
그 폴더 안의 InitOpenGL.sln을 열고 코드를 작성한다. 프로젝트 이름은 InitOpenGL로 남아 있어도 실행에 문제없다.
라이브러리 경로가 프로젝트 폴더 기준이므로 폴더 전체를 옮겨도 된다.
externals만 다른 위치로 옮기지 않는다.

## 다운로드와 라이브러리 출처

- [전체 시작 ZIP](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip): 처음부터 구성할 때 사용
- [Visual Studio 프로젝트 파일만](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/VisualStudio-project-files.zip): 예전 ZIP으로 이미 실습 폴더를 만든 경우 사용
- [externals.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/externals.zip): 라이브러리만 다시 받을 때 사용

externals는 로컬에 받아 둔 [snooy-dev/opengl-labs](https://github.com/snooy-dev/opengl-labs)의 폴더를 묶었다.
GLFW·GLEW·GLM의 기존 라이선스 파일을 포함하며, 원본 배포 바이너리를 재포장한 것이다.
config/opengl.props의 설정은 이 프로젝트 파일에 필요한 내용으로 들어 있으므로 따로 추가하지 않아도 된다.
ZIP의 .vscode, build.bat, run.bat은 다른 편집기·명령창을 위한 추가 파일이다. 위 Visual Studio 절차에는 필요 없다.

## 부록: 예전 ZIP을 이미 받은 경우

이 부분은 처음 설치하는 절차가 아니다. 처음 시작한다면 위 1번부터 7번까지만 따라가면 된다.
예전 ZIP을 받아 main.cpp와 externals는 있지만 InitOpenGL.sln과 InitOpenGL.vcxproj가 없는 경우에만 아래 방법을 사용한다.

1. 수정한 main.cpp를 Ctrl+S로 저장한다.
2. [프로젝트 파일 전용 ZIP](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/VisualStudio-project-files.zip)을 받고 압축을 푼다.
3. 안의 InitOpenGL.sln, InitOpenGL.vcxproj, PracticeStarterGL.h, CubeVertices.h, shaders 폴더를 기존 main.cpp가 있는 폴더에 복사한다. main.cpp와 externals는 유지한다.
4. Visual Studio에서 파일 → 열기 → 프로젝트/솔루션으로 방금 넣은 InitOpenGL.sln을 연다.
5. 위 5번과 6번의 화면 확인·실행 절차를 따른다.

이 프로젝트 전용 ZIP은 기존 코드를 덮어쓰지 않기 때문에 새 셰이더 로딩 C++ 코드까지 자동 적용하지는 않는다.
셰이더·카메라 초기화가 포함된 최신 초기 코드부터 시작하려면 전체 시작 ZIP을 새 폴더에 풀어 사용한다. 기존 수정 코드는 별도로 보관한다.
