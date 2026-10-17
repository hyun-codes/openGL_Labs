# Practice0 - VAO/VBO 기초 연습장

현재 코드는 강의자료를 보면서 VAO와 VBO를 처음부터 직접 작성하기 위한 기본 상태다.

현재 준비된 내용:

- GLFW 윈도우 생성
- OpenGL 3.3 Core Profile 설정
- GLEW 초기화
- vertex/fragment shader 파일 읽기, 컴파일 및 링크
- 프레임버퍼 크기 변경 처리
- 파란색 배경 출력

아직 작성하지 않은 내용:

- 정점 좌표 배열
- VAO 생성과 바인딩
- VBO 생성, 바인딩 및 데이터 저장
- `glVertexAttribPointer()` 설정
- `glDrawArrays()`를 이용한 점, 선, 삼각형 출력

`InitBuffer()`와 `drawScene()`에 남겨 둔 TODO를 강의자료 순서대로 채우면 된다.
