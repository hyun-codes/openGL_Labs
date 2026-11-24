# OpenGL Labs

C++와 OpenGL을 공부하며 작성한 실습 코드다. 실습별 코드와 프로젝트는 Practice 폴더에 있다.

**[정육면체 애니메이션 연습 4종](AnimationPractices.md)**: 로봇 이동·점프, 굴러가는 육면체, 그네, 시소를 각각 별도 프로젝트로 구현했다. AnimationPractices.sln에서 선택해 실행할 수 있다.

## Visual Studio만 있는 학교 PC에서 시작하기

**[처음부터 따라 하는 상세 README](InitOpenGL/README.md)**

새 폴더 만들기, 다운로드, 압축 해제, 폴더 구조 확인, Visual Studio에서 솔루션 열기, 실행까지 순서대로 설명되어 있다.
VS Code를 설치할 필요는 없다.

1. Visual Studio Installer에서 **C++를 사용한 데스크톱 개발**이 설치되어 있는지 확인한다.
2. [학교용 전체 ZIP](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip)을 다운로드한다.
3. C드라이브에 **Opengl_codingTest** 폴더를 만든다.
4. ZIP을 그 안에 **모두 압축 풀기**로 해제한다. `C:\Opengl_codingTest\InitOpenGL` 안에 코드·프로젝트·externals가 함께 있어야 한다.
5. Visual Studio의 **파일 → 열기 → 프로젝트/솔루션**에서 `C:\Opengl_codingTest\InitOpenGL\InitOpenGL.sln`을 연다.
6. 상단 **Debug / x64 / 로컬 Windows 디버거**를 확인한다.
7. **Ctrl+F5**로 실행한다. 디버깅은 F5 또는 ▶를 사용한다. 푸른 배경 창이 뜨면 성공이다.
8. main.cpp 수정 → Ctrl+S 저장 → Ctrl+F5 순서로 실습한다. 이전 실행 창은 닫고 다시 빌드한다.

각 단계의 클릭 위치와 폴더 구조는 [상세 README의 1번부터](InitOpenGL/README.md)에 설명되어 있다.
라이브러리 연결과 DLL 복사는 프로젝트가 자동 처리하므로 별도로 설정하지 않아도 된다.
초기 소스는 main.cpp, vertex.glsl, fragment.glsl 세 파일이며 같은 폴더에 있다. 셰이더 로딩과 Camera 구조체 및 행렬 전달은 main.cpp에 구현되어 있다.
기본 설정은 main.cpp에서 OpenGL 초기화와 셰이더 로딩을 직접 구현한다. 같은 파일에 VAO·VBO 초기화와 입력 및 그리기 함수를 추가하면 된다.
정육면체 정점 배열은 InitOpenGL/main.cpp 상단에 포함되어 있다. 같은 파일에서 VAO·VBO와 모델 그리기를 구현해 사용한다.

## 다운로드

| 파일 | 용도 |
| --- | --- |
| [OpenGL-school-starter.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip) | 셰이더·카메라 행렬 연결을 포함한 초기 코드 + Visual Studio 프로젝트 + 라이브러리 전체 |
| [VisualStudio-project-files.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/VisualStudio-project-files.zip) | 프로젝트 파일과 셰이더 파일. 기존 main.cpp는 유지 |
| [externals.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/externals.zip) | GLFW·GLEW·GLM 라이브러리만 |

다운로드한 라이브러리 ZIP에는 기존 라이선스 파일이 포함되어 있다.
InitOpenGL은 독립적으로 실행하는 초기 실습 폴더다. 기존 Practice 프로젝트는 openGL_Project1.slnx로 연다.
