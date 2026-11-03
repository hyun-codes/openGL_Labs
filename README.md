# OpenGL Labs

C++와 OpenGL을 공부하며 작성한 실습 코드이다.
실습별 코드와 프로젝트는 Practice 폴더에 있다.

## 학교 PC에서 새 실습 폴더 만들기

**[단계별 설치·폴더 구성·빌드 안내](InitOpenGL/README.md)**

| 다운로드 | 용도 |
| --- | --- |
| [OpenGL-school-starter.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/OpenGL-school-starter.zip) | 초기 코드 + 빌드·실행 설정 + externals 전체. 처음 시작할 때 권장 |
| [externals.zip](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/downloads/externals.zip) | GLFW·GLEW·GLM만 받기. 기존 실습 폴더에 externals 폴더를 넣을 때 사용 |

시작 ZIP을 압축 해제한다. Visual Studio에서는 **InitOpenGL.sln**을 열고 **Ctrl+F5 / F5**로 실행한다.
VS Code에서는 InitOpenGL 폴더를 연다.
Visual Studio 2022/2026의 **C++를 사용한 데스크톱 개발** 도구가 설치된 Windows에서 아래 순서로 실행한다.

```powershell
.\build.bat
.\run.bat
```

다운로드한 ZIP 내부에는 라이브러리 라이선스가 포함되어 있다.
InitOpenGL은 이동 가능한 VS Code 시작 환경이며 기존 Practice 프로젝트의 절대 경로 설정을 변경하지 않는다.
기존 Practice 프로젝트는 Visual Studio에서 openGL_Project1.slnx로 연다.
