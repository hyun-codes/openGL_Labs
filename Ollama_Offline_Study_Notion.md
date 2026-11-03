# Ollama 오프라인 OpenGL 실습 가이드

이 문서는 **학교 Windows PC에서 Visual Studio로 실습하고, PowerShell로 로컬 Ollama에 내 코드 설명을 요청하는 방법**을 처음부터 정리한 것이다.

사용 모델: **qwen2.5-coder:3b**

이 파일은 Notion으로 가져올 수 있는 Markdown이다. GitHub에서 그대로 읽어도 되고, 파일을 다운로드한 뒤 Notion의 **가져오기(Import)**에서 Markdown 가져오기 항목을 선택해 넣어도 된다.

## 1. 먼저 이해할 것

```text
Visual Studio에서 코드 작성·저장
    → PowerShell이 C++·셰이더 파일을 읽음
    → 내 PC의 Ollama 서버에 코드와 질문 전달
    → qwen2.5-coder:3b가 답변
    → 내가 답을 확인하고 Visual Studio에서 수정·실행
```

- PowerShell 자체가 AI는 아니다. 파일을 읽어 Ollama에 전달하는 역할이다.
- `http://localhost:11434`의 localhost는 **내 컴퓨터**라는 뜻이다. 외부 인터넷 사이트가 아니다.
- 모델 다운로드와 프로그램 설치를 미리 마쳤다면 **와이파이가 꺼져도 질문할 수 있다.**
- 폴더 경로만 AI에게 말하는 것으로는 파일을 읽을 수 없다. 이 문서의 스크립트가 실제 파일 내용을 전달한다.
- 코드를 읽고 답변할 뿐 자동으로 파일을 수정하거나 컴파일하지 않는다.
- 내가 집 PC에 설치한 모델이 학교 PC에 자동으로 설치되는 것은 아니다. **사용할 학교 PC에서도 미리 설치·다운로드해야 한다.**

## 2. 인터넷이 연결되어 있을 때 Ollama 설치

1. [Ollama Windows 다운로드](https://ollama.com/download/windows)를 연다.
2. 설치 파일을 다운로드하고 설치한다.
3. 설치가 끝나면 Ollama 앱을 실행한다.
4. Windows 시작 메뉴에서 **PowerShell**을 검색해 실행한다.
5. 다음 명령을 입력한다.

```powershell
ollama --version
```

버전 정보가 나오면 명령이 인식된 것이다. 서버 연결 경고가 함께 나오면 Ollama 앱을 실행하고 다시 확인한다.
`ollama`를 찾을 수 없다고 나오면 설치 후 PowerShell을 닫고 새로 연다.

학교 PC의 프로그램 설치가 제한되어 있다면 먼저 담당자에게 설치 가능 여부를 확인한다.

## 3. 사용할 모델 다운로드

**이 단계는 인터넷이 연결되어 있을 때 한 번 진행한다.**

```powershell
ollama pull qwen2.5-coder:3b
```

다운로드가 끝나면 설치 목록을 확인한다.

```powershell
ollama list
```

NAME 열에 다음 이름이 있어야 한다.

```text
qwen2.5-coder:3b
```

이 문서에서 이름은 콜론 뒤 `3b`까지 그대로 사용한다.
모델 파일은 Ollama가 관리한다. 실습의 externals 폴더에 넣는 파일이 아니다.

## 4. 모델만 먼저 실행해보기

```powershell
ollama run qwen2.5-coder:3b
```

대화 입력 화면이 나오면 다음과 같이 질문한다.

```text
OpenGL에서 VAO와 VBO의 차이를 한국어로 짧게 설명해줘.
```

답변이 나오는지 확인한 다음 아래 명령으로 대화에서 나온다.

```text
/bye
```

이 기본 대화창은 내 프로젝트 폴더를 자동으로 읽지 않는다.
아래 단계에서는 PowerShell 스크립트가 파일을 읽어 모델에 직접 질문하므로 `ollama run` 대화창을 따로 열 필요 없다.

## 5. 학교 PC에서 사용할 파일 미리 받기

인터넷이 될 때 아래 스크립트도 다운로드한다.

**[Ask-OpenGL.ps1 다운로드](https://github.com/hyun-codes/openGL_Labs/raw/refs/heads/master/Ask-OpenGL.ps1)**

1. C드라이브에 **OllamaStudy** 폴더를 만든다.
2. 다운로드한 스크립트를 `C:\OllamaStudy`로 옮긴다.
3. 파일 이름이 **Ask-OpenGL.ps1**인지 확인한다. `.ps1.txt`가 붙으면 안 된다.

브라우저에서 다운로드 대신 코드가 보이면 Ctrl+S로 저장한다. 파일 형식은 **모든 파일**, 이름은 **Ask-OpenGL.ps1**로 지정한다.

학교용 OpenGL 초기 환경은 별도로 [InitOpenGL README](https://github.com/hyun-codes/openGL_Labs/blob/master/InitOpenGL/README.md)의 처음 단계부터 준비한다.
이미 Visual Studio에서 Ctrl+F5로 실습이 실행된다면 다시 만들 필요 없다.

이 문서는 다음 폴더 구성을 예로 든다.

```text
C:\
├─ OllamaStudy\
│  └─ Ask-OpenGL.ps1          ← 코드 읽기·질문용 스크립트
└─ Opengl_codingTest\
   └─ InitOpenGL\            ← Visual Studio 프로젝트
      ├─ InitOpenGL.sln
      ├─ InitOpenGL.vcxproj
      ├─ main.cpp
      ├─ PracticeStarterGL.h
      ├─ CubeVertices.h
      ├─ shaders\
      │  ├─ vertex.glsl
      │  └─ fragment.glsl
      └─ externals\
```

**여기까지 인터넷이 있을 때 준비해둔다.** 이후 질문은 로컬 모델을 사용한다.

## 6. 와이파이를 끈 상태에서 Ollama 준비

1. Ollama 앱을 실행한다.
2. PowerShell에서 다음을 입력한다.

```powershell
ollama list
```

`qwen2.5-coder:3b`가 나오면 된다.

서버 연결 오류가 나오거나 앱을 실행할 수 없다면 별도의 PowerShell 창을 하나 열어 아래를 입력하고 그 창을 켜둔다.

```powershell
ollama serve
```

그 후 **다른 PowerShell 창에서** 아래 질문 절차를 진행한다.
이미 11434 포트를 사용 중이라는 메시지가 나오면 서버가 실행 중일 수 있으므로 중복으로 띄우지 말고 `ollama list`로 확인한다.

## 7. Visual Studio에서 저장하기

1. Visual Studio에서 실제 실습의 `.sln`을 연다.
2. 실습 코드를 작성한다.
3. **Ctrl+Shift+S**로 모두 저장한다.

Ollama에 전달되는 것은 디스크에 저장된 파일이다. 저장하지 않은 편집 내용은 반영되지 않는다.
Visual Studio를 켜둔 채 PowerShell을 사용해도 된다.

## 8. PowerShell에서 질문 준비

PowerShell을 열고 다음 두 명령을 입력한다.

```powershell
cd C:\OllamaStudy
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
```

두 번째 명령은 **현재 PowerShell 창에서만** 스크립트 실행을 허용한다. 창을 닫으면 유지되지 않는다.
학교의 관리 정책으로 실행이 제한된 경우 이 명령으로도 허용되지 않을 수 있다.

파일이 있는지 확인한다.

```powershell
dir .\Ask-OpenGL.ps1
```

## 9. 전체 C++·셰이더 코드 읽히고 질문하기

아래 명령을 그대로 입력한다. 줄 끝의 백틱(`) 뒤에는 공백을 넣지 않는다.

```powershell
.\Ask-OpenGL.ps1 `
    -ProjectPath "C:\Opengl_codingTest\InitOpenGL" `
    -Question "전체 코드의 실행 흐름을 함수별로 한국어로 설명해줘."
```

줄바꿈이 어려우면 한 줄로 입력해도 된다.

```powershell
.\Ask-OpenGL.ps1 -ProjectPath "C:\Opengl_codingTest\InitOpenGL" -Question "전체 코드의 실행 흐름을 함수별로 한국어로 설명해줘."
```

스크립트는 다음 작업을 한다.

1. 내 PC의 Ollama 서버와 모델 설치 여부를 확인한다.
2. 지정한 폴더 아래 C++·헤더·셰이더 파일을 찾는다.
3. **실제로 읽는 파일 목록을 PowerShell에 표시한다.**
4. 파일 이름과 내용을 모아서 질문과 함께 Ollama에 보낸다.
5. 모델이 만든 한국어 답변을 출력한다.

기본 모델은 qwen2.5-coder:3b라서 명령마다 모델 이름을 적을 필요 없다.
읽는 확장자는 cpp/cc/cxx/h/hpp/glsl/vert/frag이고, externals/bin/obj/.vs/.git/node_modules 안의 파일은 제외한다.
라이브러리 전체나 실행 파일을 모델에 넣지 않는다.

처음에는 모델 로딩으로 시간이 걸릴 수 있다. 답변을 다 만든 다음 한 번에 표시하는 방식이다.

## 10. 실습 중 반복해서 사용하는 순서

```text
Visual Studio에서 코드 작성
    → Ctrl+Shift+S로 모두 저장
    → PowerShell에서 Ask-OpenGL.ps1로 질문
    → 설명이나 수정안을 내가 확인
    → Visual Studio에서 직접 수정
    → Ctrl+S
    → Ctrl+F5로 빌드·실행
```

질문할 때마다 **현재 저장된 파일을 다시 읽는다.** 파일을 바꾼 후 별도의 재등록 작업은 필요 없다.
이전 질문의 대화를 이어서 기억하는 방식은 아니다. 매번 현재 코드와 질문을 새로 보낸다.

### 코드 설명 받기

```powershell
.\Ask-OpenGL.ps1 -ProjectPath "C:\Opengl_codingTest\InitOpenGL" -Question "DrawScene에서 모델, 뷰, 투영 행렬이 어떤 순서로 적용되는지 내 코드를 기준으로 설명해줘."
```

### 충돌이나 애니메이션 점검

```powershell
.\Ask-OpenGL.ps1 -ProjectPath "C:\Opengl_codingTest\InitOpenGL" -Question "회전 중심과 행렬 곱 순서에 잘못된 부분이 있는지 확인해줘. 수정이 필요한 함수만 설명해줘."
```

### 빌드 오류 질문

```powershell
.\Ask-OpenGL.ps1 -ProjectPath "C:\Opengl_codingTest\InitOpenGL" -Question "빌드 오류 C2065가 나와. 오류 메시지는 여기에 붙여넣을게: 실제 오류 메시지. 원인과 최소 수정안을 알려줘."
```

실제 오류 메시지를 함께 넣어야 한다. 모델이 Visual Studio 오류 창이나 실행 화면을 자동으로 보는 것은 아니다.

## 11. 다음 날 다시 사용할 때

이미 설치한 프로그램과 모델을 다시 다운로드하지 않는다.

1. Ollama 앱을 실행한다. 필요하면 다른 창에서 `ollama serve`를 실행한다.
2. Visual Studio에서 실습 솔루션을 열고 파일을 저장한다.
3. PowerShell을 열고 다음을 입력한다.

```powershell
cd C:\OllamaStudy
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\Ask-OpenGL.ps1 -ProjectPath "C:\Opengl_codingTest\InitOpenGL" -Question "현재 구현된 기능과 아직 비어 있는 부분을 정리해줘."
```

다른 실습 폴더를 사용하면 `-ProjectPath`만 해당 폴더로 바꾼다.
모든 Practice 폴더가 들어 있는 큰 저장소 전체보다 **지금 진행하는 실습 폴더 하나**를 지정하는 것이 좋다.

## 12. 자주 생기는 문제

| 상황 | 확인할 것 |
| --- | --- |
| ollama 명령을 찾을 수 없음 | Ollama 설치 후 PowerShell을 새로 연다. |
| Ollama is not reachable | Ollama 앱 또는 다른 창의 ollama serve 실행 상태를 확인한다. |
| Model is not installed | 인터넷이 될 때 이 PC에서 ollama pull qwen2.5-coder:3b를 실행한다. |
| 스크립트를 실행할 수 없다는 보안 오류 | 8번의 현재 창 실행 정책 명령을 확인한다. 조직 정책에 의한 제한이면 담당자에게 확인한다. |
| Ask-OpenGL.ps1을 찾을 수 없음 | C:\OllamaStudy 안에 스크립트가 있는지, 확장자가 ps1.txt인지 확인한다. |
| ProjectPath 경로가 없다는 오류 | 탐색기 주소와 -ProjectPath가 같은지 확인한다. 공백이 있는 경로도 따옴표로 감싼다. |
| 읽은 파일 목록이 이상함 | 프로젝트 폴더를 너무 상위로 지정하지 않았는지 확인한다. |
| 수정 전 코드를 설명함 | Visual Studio에서 Ctrl+Shift+S로 저장한 뒤 다시 질문한다. |
| 답변이 느리거나 메모리 오류 | 다른 프로그램의 메모리 사용량을 확인한다. 아래처럼 ContextLength를 낮춰볼 수 있다. |
| 코드 일부를 빠뜨리거나 엉뚱한 답을 함 | 3b 모델의 한계 또는 문맥 한도일 수 있다. 질문을 좁히고 필요한 코드만 있는 폴더로 줄인다. |

메모리 부담을 줄이는 예:

```powershell
.\Ask-OpenGL.ps1 -ProjectPath "C:\Opengl_codingTest\InitOpenGL" -ContextLength 8192 -Question "main 함수의 실행 순서를 설명해줘."
```

문맥 크기를 줄이면 한 번에 읽을 수 있는 코드 양도 줄어든다.
기본값 16384가 모든 프로젝트를 반드시 담는다는 뜻은 아니다. 모델은 긴 코드에서 앞부분을 놓치거나 실수할 수 있다.
스크립트가 파일 내용을 전달했더라도 답변의 정확성까지 보장하지는 않는다.

## 13. 종료하기

- 질문이 끝나면 PowerShell을 닫아도 된다.
- 직접 실행한 ollama serve 창을 종료하려면 그 창에서 Ctrl+C를 누른다. 다른 프로그램도 사용 중이라면 서버를 유지한다.
- 필요하면 다음 명령으로 메모리에 올라온 모델을 내릴 수 있다. 설치된 모델 파일을 삭제하는 명령은 아니다.

```powershell
ollama stop qwen2.5-coder:3b
```

## 참고

- [Ollama 공식 다운로드](https://ollama.com/download/windows)
- [Ollama Windows 문서](https://docs.ollama.com/windows)
- [Ollama CLI 문서](https://docs.ollama.com/cli)
- [Ollama Chat API 문서](https://docs.ollama.com/api/chat)
- [이 저장소의 질문 스크립트](https://github.com/hyun-codes/openGL_Labs/blob/master/Ask-OpenGL.ps1)

스크립트는 localhost에만 요청하고 파일을 읽기만 한다. 프로젝트 파일을 자동으로 변경하지 않는다.
