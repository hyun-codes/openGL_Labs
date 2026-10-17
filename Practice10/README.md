# Practice10

VAO와 VBO를 사용하는 작은 도형 모양 맞추기 실습이다.

- 왼쪽: 무작위 위치와 색상의 작은 사각형, 정삼각형, 직각삼각형
- 오른쪽: 기본 모양판 3개와 추가 모양판 2개
- 마우스 왼쪽 드래그: 작은 도형 이동
- 같은 종류의 빈 슬롯 가까이에서 놓기: 슬롯에 맞춰서 배치
- 모양판 완성: 해당 모양판의 모든 도형이 잠김
- R: 전체 랜덤 초기화
- Q 또는 Esc: 종료

## VAO/VBO 흐름

`Piece/Pattern -> vertices -> glBufferData -> VBO -> VAO의 location 0/1 -> vertex.glsl -> glDrawArrays`
