# 정육면체 애니메이션 연습 4종

각 예제는 별도 main.cpp·Visual Studio 프로젝트·솔루션·공통 헤더·셰이더를 갖는다.
기능마다 // 한국어 주석을 달았으며 InitBuffer → Update → DrawScene 구조를 사용한다.

모든 예제는 23번처럼 여섯 면으로 이루어진 방 안에 배치했다. 각 벽의 안쪽이 정면이 되도록 만들어 뒷면 컬링을 적용했으므로 카메라 이동·공전에 따라 보이는 벽이 바뀐다.
방의 x/z 범위는 -6~6이고 y 범위는 0~6이다. 바닥 격자도 방 안에만 그린다.

| 폴더 | 기능 | 입력 |
| --- | --- | --- |
| RobotPractice | 정육면체 로봇, 방향키 이동, 관절 걷기, 점프 | 방향키 / Space 점프 |
| RollingCubePractice | 접촉 모서리를 축으로 90도씩 구르기 | WASD 이동 / Space 정지·재개 |
| SwingPractice | 그네와 같은 방의 이동·점프 로봇 | Space 그네 정지·재개 / 방향키 로봇 이동 / J 점프 |
| SeesawPractice | 시소와 같은 방의 이동·점프 로봇 | Space 시소 정지·재개 / 방향키 로봇 이동 / J 점프 |

공통 입력: C 또는 I 초기화, ESC 또는 Q 종료.

## 네 예제 공통 카메라 명령

| 키 | 기능 |
| --- | --- |
| x / Shift+x | 월드 x축 양/음 방향 이동 |
| z / Shift+z | 월드 z축 양/음 방향 이동 |
| y / Shift+y | 제자리 양방향 회전. 같은 방향을 다시 누르면 정지 |
| r / Shift+r | 장면 중심 y축 주위로 양방향 공전. 같은 방향을 다시 누르면 정지 |
| + / - | 줌인/줌아웃. = 키와 숫자패드도 지원 |
| p / Shift+p | 원근 투영/직각 투영 |

기존 19번과 같은 카메라 자전·공전 키를 사용한다. R을 공전에 쓰므로 초기화는 C/I이다.
객체의 방향키·Space 입력과 카메라가 동시에 동작한다. Space로 객체를 멈춰도 카메라는 멈추지 않는다.

## Visual Studio에서 실행

AnimationPractices.sln을 열면 4개 프로젝트가 함께 보인다.
실행할 프로젝트를 솔루션 탐색기에서 우클릭 → 시작 프로젝트로 설정하고 Debug / x64 → Ctrl+F5로 실행한다.
각 폴더 안의 개별 sln을 열어 하나만 실행해도 된다.
기존 openGL_Project1.slnx에도 네 프로젝트를 추가했다.

## 학교 PC에서 실행할 파일

downloads/AnimationPractices-school.zip을 받아 압축을 풀면 다음 구조다.

```text
AnimationPractices/
├─ AnimationPractices.sln
├─ AnimationPractices.md
├─ externals/                 ← GLFW·GLEW·GLM 전체
├─ RobotPractice/
├─ RollingCubePractice/
├─ SwingPractice/
└─ SeesawPractice/
```

전체 폴더를 옮겨도 되고, 개별 폴더만 옮길 때는 그 폴더 안에 externals를 추가하면 된다.
프로젝트는 자기 폴더의 externals, 상위 externals, 상위 InitOpenGL/externals 순으로 사용 가능한 경로를 찾는다.
현재 개발 PC의 기존 D:/OpenGL 설치 경로도 마지막 대안으로 지원한다.
학교 PC는 Visual Studio 2022 또는 2026의 C++ 데스크톱 개발·Windows SDK가 필요하다.
압축 안에는 모델 파일이나 외부 물리 엔진이 필요 없는 코드와 라이브러리가 포함되어 있다.

## 다른 초기 프로젝트에 main.cpp를 복사할 때

main.cpp 전체를 교체하고 해당 폴더의 PracticeStarterGL.h와 shaders 폴더도 함께 가져간다.
include는 같은 폴더 기준인 #include "PracticeStarterGL.h"를 사용한다.
초기 코드와 합쳐서 붙이지 않는다. main·Camera·shaderProgram 등의 이름이 중복될 수 있기 때문이다.
