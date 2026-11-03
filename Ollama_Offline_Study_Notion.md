# Ollama 설치와 오프라인 실행

Windows PowerShell에서 **qwen2.5-coder:3b**를 실행하는 방법이다.
프로그램 설치와 모델 다운로드는 인터넷이 연결되어 있을 때 미리 진행한다.
완료한 뒤에는 인터넷 없이 설치된 모델과 대화할 수 있다.

## 1. Ollama 설치

1. [Ollama Windows 다운로드](https://ollama.com/download/windows)에서 설치 파일을 받는다.
2. 설치를 완료하고 Ollama 앱을 실행한다.
3. PowerShell을 새로 열고 다음을 입력한다.

```powershell
ollama --version
```

버전 정보가 나오면 명령이 인식된 것이다.
ollama를 찾을 수 없다고 나오면 설치 후 PowerShell을 닫았다가 다시 연다.

## 2. 모델 다운로드

인터넷이 연결된 상태에서 입력한다. 사용할 PC마다 한 번씩 다운로드해야 한다.

```powershell
ollama pull qwen2.5-coder:3b
```

다운로드가 완료되면 설치 목록을 확인한다.

```powershell
ollama list
```

NAME 열에 **qwen2.5-coder:3b**가 있으면 준비가 완료된 것이다.

## 3. 모델 실행

Ollama 앱을 켜고 PowerShell에서 입력한다.

```powershell
ollama run qwen2.5-coder:3b
```

입력 화면이 나오면 질문을 입력한다. 코드는 Visual Studio에서 복사해 대화 입력란에 붙여넣고 질문하면 된다.
프로젝트 폴더를 지정하거나 별도의 ps1 파일을 만들 필요 없다.
모델이 폴더를 자동으로 읽는 것은 아니므로 필요한 코드와 오류 메시지를 직접 붙여넣는다.

여러 줄 코드를 한 번의 메시지로 입력하려면 아래처럼 **큰따옴표 세 개**로 감싼다.
이 부분은 PowerShell 명령이 아니라 모델이 실행된 대화창에서 입력한다.

```text
"""
다음 코드를 보고 오류 원인을 설명해줘.

여기에 코드 붙여넣기
"""
```

## 4. 종료하고 다시 실행하기

대화 입력란에서 다음을 입력하면 PowerShell로 돌아온다.

```text
/bye
```

다음에 사용할 때는 모델을 다시 다운로드하지 않고 실행 명령만 입력한다.

```powershell
ollama run qwen2.5-coder:3b
```

## 서버 연결 오류가 나올 때만

Ollama 앱을 먼저 실행한다.
그래도 연결되지 않으면 별도 PowerShell 창에서 아래 명령을 실행하고 그 창을 켜둔다.

```powershell
ollama serve
```

다른 PowerShell 창에서 모델을 실행한다.

```powershell
ollama run qwen2.5-coder:3b
```

이미 11434 포트가 사용 중이라는 메시지가 나오면 서버가 실행 중일 수 있으므로 중복 실행하지 않는다.
직접 실행한 서버를 종료하려면 해당 창에서 Ctrl+C를 누른다.

## 오프라인 사용 조건

- 사용할 PC에 Ollama와 모델을 미리 설치·다운로드해둔다.
- 모델이 설치되어 있으면 위 실행·질문 과정에는 와이파이가 필요 없다.
- 모델 답변을 Visual Studio에 적용하고 직접 빌드·실행해서 확인한다. 파일 자동 수정 기능은 없다.
