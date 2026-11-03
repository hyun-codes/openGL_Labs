# 학교 PC에서 Visual Studio로 OpenGL 실습 시작하기

**이 문서는 Visual Studio만 설치된 학교 PC 기준이다. VS Code는 필요 없다.**

목표: `C:\Opengl_codingTest` 폴더 하나를 만들고, 준비된 파일을 넣은 다음 Visual Studio의 **Ctrl+F5 / F5 / ▶**로 빌드하고 실행한다.

처음부터 준비하는 경우 **1번부터 순서대로** 따라간다.
이미 압축을 풀고 main.cpp까지 열었는데 **구성 없음**이 보이는 경우 바로 아래에서 이어간다.

## 지금 하던 상태에서 이어가기

현재 폴더가 다음과 같고 Visual Studio에서 main.cpp를 보고 있는 상황이다.

```text
C:\Opengl_codingTest\InitOpenGL\
├─ main.cpp
├─ build.bat
├─ run.bat
├─ externals\
└─ ...
```

위쪽에 **구성 없음 / 시작 항목 선택**이 보이는 이유는 프로젝트가 아니라 폴더 또는 파일만 열었기 때문이다.
처음 배포한 ZIP에는 Visual Studio 프로젝트가 빠져 있었다. 이 경우 아래 두 파일만 추가하면 된다.
**기존 main.cpp와 externals는 그대로 사용한다. 다시 만들 필요 없다.**

1. main.cpp를 수정했다면 **Ctrl+S**로 먼저 저장한다.
2. **[Visual Studio 프로젝트 파일만 다운로드](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/VisualStudio-project-files.zip)**를 누른다.
3. 받은 ZIP을 마우스 오른쪽으로 클릭하고 **모두 압축 풀기**를 선택한다.
4. 압축을 푼 곳의 **InitOpenGL.sln**, **InitOpenGL.vcxproj** 두 파일을 복사한다.
5. 두 파일을 **main.cpp가 있는 바로 그 폴더**, 즉 `C:\Opengl_codingTest\InitOpenGL`에 붙여넣는다.
6. 아래처럼 파일이 함께 있는지 확인한다.

```text
C:\Opengl_codingTest\InitOpenGL\
├─ InitOpenGL.sln        ← 이번에 추가
├─ InitOpenGL.vcxproj    ← 이번에 추가
├─ main.cpp             ← 기존 파일 유지
├─ build.bat
├─ run.bat
└─ externals\           ← 기존 폴더 유지
```

7. **현재 Visual Studio에서 파일 → 열기 → 프로젝트/솔루션**을 선택한다.
8. `C:\Opengl_codingTest\InitOpenGL\InitOpenGL.sln`을 선택하고 **열기**를 누른다.
9. 위쪽이 **Debug / x64 / 로컬 Windows 디버거**로 바뀌는지 확인한다.
10. **Ctrl+F5**를 누른다. 빌드할지 묻는 창이 나오면 **예**를 누른다.

푸른 배경 창이 뜨면 완료다. ESC 또는 창의 X로 종료한다.
자세한 화면 확인은 아래 **5번과 6번**을 보면 된다.

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
└─ glfw3.dll
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

초기 코드의 흐름은 다음과 같다.

```text
main
  → InitOpenGL: GLFW·윈도우·컨텍스트·GLEW·뷰포트 초기화
  → 반복: DrawScene → 버퍼 교체 → 이벤트 처리
  → Cleanup: 윈도우와 GLFW 정리
```

- 현재 초기 코드에는 도형을 그리는 부분이 없다.
- 이후 셰이더와 정점·VAO·VBO 초기화 함수를 추가한다.
- 초기화 함수는 OpenGL 컨텍스트와 GLEW 초기화가 성공한 뒤, 메인 반복문 전에 호출한다.
- 도형을 그리는 호출은 DrawScene에 넣는다.
- OpenGL 3.3 Core 설정이므로 glRectf/glBegin 대신 셰이더와 VAO·VBO 방식을 사용한다.
- 별도 cpp 파일을 만들 때는 프로젝트 우클릭 → **추가 → 새 항목 → C++ 파일**로 추가한다. 탐색기에 파일을 복사하기만 하면 프로젝트에 자동으로 포함되지 않을 수 있다.
- 셰이더를 `shaders` 폴더에 넣었다면 `shaders/vertex.glsl`처럼 프로젝트 폴더 기준으로 읽을 수 있다. 실행 작업 경로는 InitOpenGL.vcxproj가 있는 폴더다.

## 9. 문제가 생겼을 때

| 상황 | 확인할 내용 |
| --- | --- |
| 구성 없음 / 시작 항목 선택 | 파일 → 열기 → 프로젝트/솔루션 → InitOpenGL.sln을 연다. main.cpp만 열거나 폴더만 연 상태가 아닌지 확인한다. |
| InitOpenGL.sln이 없음 | 예전 ZIP일 수 있다. 문서 위쪽의 프로젝트 파일 전용 ZIP을 받아 두 파일을 기존 main.cpp 옆에 넣는다. |
| 시작 프로젝트를 선택하라는 메시지 | 솔루션 탐색기에서 InitOpenGL 프로젝트 우클릭 → 시작 프로젝트로 설정 |
| C++ 빌드 도구가 없다는 메시지 | 1번의 Visual Studio Installer에서 C++를 사용한 데스크톱 개발 설치 |
| Windows SDK를 찾을 수 없음 | Visual Studio Installer에서 Windows SDK 설치 후 솔루션을 다시 연다. |
| glew.h / glfw3.h / glm.hpp를 찾을 수 없음 | externals와 main.cpp가 같은 폴더 아래인지, externals가 중복되지 않았는지 4번 구조와 비교한다. |
| glew32.lib / glfw3dll.lib를 찾을 수 없음 | x64 설정인지, externals의 lib·lib-vc2022·lib-vc2026 폴더가 있는지 확인한다. |
| LNK1104로 OpenGLStudy.exe를 열 수 없음 | 이전에 실행한 OpenGL 창을 종료한 후 다시 빌드한다. |
| DLL이 없다는 메시지 | 빌드가 성공했는지 확인하고, bin\VisualStudio\Debug 안에 glew32.dll과 glfw3.dll이 있는지 확인한다. |
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
