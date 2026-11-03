# 정육면체 정점 사용하기

CubeVertices.h에는 15·16번에서 사용한 정육면체의 좌표·색상이 들어 있다.
정점 파일만 추가한 것이므로 초기 main.cpp에서 도형을 자동으로 그리지는 않는다.
로봇·시소를 만들 때 같은 VAO·VBO를 재사용하고, 부위마다 다른 모델 행렬로 여러 번 그리면 된다.

## 1. 파일 넣기

최신 학교용 전체 ZIP에는 main.cpp 옆에 CubeVertices.h가 들어 있다.
기존 폴더를 사용한다면 이 헤더 파일만 main.cpp 옆에 다운로드해 넣는다.

```cpp
#include "CubeVertices.h"
```

CubeVertex 구조체를 따로 정의해두었으므로 기존 Vertex 구조체와 함께 사용할 수 있다.
구조는 위치 float 3개, 색상 float 3개다. 헤더 내용을 main.cpp에 복사하는 대신 include하면 된다.

## 2. 전역 변수와 함수 선언 추가

main 함수 위에 추가한다. 같은 이름이 이미 있다면 기존 것을 사용한다.

```cpp
GLuint cubeVAO = 0;
GLuint cubeVBO = 0;

void InitCubeBuffer();
void DrawCubeScene(const glm::mat4& objectTransform);
```

## 3. InitCubeBuffer 정의

main 아래에 정의한다. 로컬 cubeVertices 배열을 다시 만들 필요 없다.

```cpp
void InitCubeBuffer()
{
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CubeVertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CubeVertex), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
```

main에서 InitOpenGL 성공 후, 반복문에 들어가기 전에 **InitCubeBuffer();**를 한 번 호출한다.

## 4. DrawCubeScene 정의

```cpp
void DrawCubeScene(const glm::mat4& objectTransform)
{
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "modelTransform"),
        1, GL_FALSE, glm::value_ptr(objectTransform)
    );
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, cubeVertexCount);
    glBindVertexArray(0);
}
```

초기 코드의 DrawScene에서 glUseProgram과 카메라·투영 uniform 전달이 끝난 뒤 아래 코드를 추가한다.
기존의 배경 지우기·glUseProgram·행렬 전달 부분을 지우지 않는다.

```cpp
glm::mat4 objectTransform(1.0f);
objectTransform = glm::rotate(objectTransform, glm::radians(30.0f), glm::vec3(1, 0, 0));
objectTransform = glm::rotate(objectTransform, glm::radians(30.0f), glm::vec3(0, 1, 0));
DrawCubeScene(objectTransform);
```

종료할 때는 기존 Cleanup 함수의 윈도우 삭제보다 앞에서 버퍼도 해제한다.

```cpp
glDeleteBuffers(1, &cubeVBO);
glDeleteVertexArrays(1, &cubeVAO);
```

## 좌표·면 번호

중심은 원점이고 x/y/z 범위는 각각 -0.3~0.3이다. 기본 한 변의 길이는 0.6이다.
원하는 실제 크기가 (width,height,depth)라면 스케일은 (width/0.6,height/0.6,depth/0.6)이다.
색상은 정점 사이에서 보간된다. GL_CCW / GL_BACK 컬링을 사용할 수 있다.

| faceIndex | 면 | glDrawArrays의 first | count |
| --- | --- | --- | --- |
| 0 | 앞면, -z | 0 | 6 |
| 1 | 뒷면, +z | 6 | 6 |
| 2 | 왼쪽, -x | 12 | 6 |
| 3 | 오른쪽, +x | 18 | 6 |
| 4 | 위, +y | 24 | 6 |
| 5 | 아래, -y | 30 | 6 |

전체는 정점 36개이며 EBO는 필요 없다.
한 면만 그릴 때는 VAO 연결 후 glDrawArrays(GL_TRIANGLES, faceIndex * cubeVerticesPerFace, cubeVerticesPerFace)를 사용한다.
glDrawArrays의 first와 count는 색상 float까지 합친 개수가 아니라 **정점 개수** 기준이다.

CubeVertices.h의 cubeVertices는 constexpr 배열이므로 원본 좌표를 직접 바꾸지 않고 모델 행렬로 변환한다.
