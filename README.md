# OpenGL Labs

C++와 OpenGL을 공부하며 작성한 실습 코드다. 실습별 코드와 프로젝트는 Practice 폴더에 있다.

## Visual Studio만 있는 학교 PC에서 시작하기

**[처음부터 따라 하는 상세 README](InitOpenGL/README.md)**

새 폴더 만들기, 다운로드, 압축 해제, 폴더 구조 확인, Visual Studio에서 솔루션 열기, 실행까지 순서대로 설명되어 있다.
VS Code를 설치할 필요는 없다.

1. [학교용 전체 ZIP](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip)을 다운로드한다.
2. `C:\Opengl_codingTest` 폴더를 만들고 ZIP을 그 안에 압축 해제한다.
3. `C:\Opengl_codingTest\InitOpenGL\InitOpenGL.sln`을 Visual Studio에서 연다.
4. **Debug / x64**를 확인하고 **Ctrl+F5**로 실행한다. 디버깅은 F5 또는 ▶를 사용한다.

Visual Studio Installer의 **C++를 사용한 데스크톱 개발**이 설치되어 있어야 한다.
라이브러리 연결과 DLL 복사는 프로젝트가 자동 처리한다.

## 이미 main.cpp를 열었는데 구성 없음이 보인다면

예전 ZIP에는 Visual Studio 프로젝트가 없었다.
**[프로젝트 파일 전용 ZIP](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/VisualStudio-project-files.zip)**을 받아 압축을 풀고,
InitOpenGL.sln과 InitOpenGL.vcxproj를 **기존 main.cpp 옆에** 넣는다.
기존 코드와 externals는 유지한 채 **파일 → 열기 → 프로젝트/솔루션**으로 InitOpenGL.sln을 연다.
[현재 상태에서 이어가는 상세 단계](InitOpenGL/README.md#지금-하던-상태에서-이어가기)를 참고한다.

## 다운로드

| 파일 | 용도 |
| --- | --- |
| [OpenGL-school-starter.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip) | 초기 코드 + Visual Studio 솔루션·프로젝트 + 라이브러리 전체 |
| [VisualStudio-project-files.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/VisualStudio-project-files.zip) | 이미 압축을 푼 사용자에게 필요한 프로젝트 파일 두 개만 |
| [externals.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/externals.zip) | GLFW·GLEW·GLM 라이브러리만 |

다운로드한 라이브러리 ZIP에는 기존 라이선스 파일이 포함되어 있다.
InitOpenGL은 독립적으로 실행하는 초기 실습 폴더다. 기존 Practice 프로젝트는 openGL_Project1.slnx로 연다.
