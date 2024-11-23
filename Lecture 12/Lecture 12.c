#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include "ElfMath.h"

#define SCREEN_WIDTH 40
#define SCREEN_HEIGHT 24
#define CENTER_X 20
#define CENTER_Y 12

char screenBuffer[SCREEN_HEIGHT][SCREEN_WIDTH * 2 + 1];
clock_t startTime;
float elapsedTime;

// 각도 저장 변수들
float sunRotation = 0.0f;
float earthRotation = 0.0f;
float earthOrbit = 0.0f;
float moonRotation = 0.0f;
float moonOrbit = 0.0f;

void clearScreen() {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH * 2; x++) {
            screenBuffer[y][x] = ' ';
        }
        screenBuffer[y][SCREEN_WIDTH * 2] = '\0';
    }
}

void drawPixel(int x, int y, const char* ch) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        screenBuffer[y][x * 2] = ch[0];
        screenBuffer[y][x * 2 + 1] = ch[1];
    }
}

void renderScreen() {
    system("cls");
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        printf("%s\n", screenBuffer[y]);
    }
}

void drawTriangle(Vector3 center, float size, const char* ch, float angle) {
    Matrix3x3 rotMat = rotation_matrix(angle);

    Vector3 points[3] = {
        {0, -size, 1},  // 상단
        {-size * 0.866f, size * 0.5f, 1},  // 좌하단
        {size * 0.866f, size * 0.5f, 1}   // 우하단
    };

    for (int i = 0; i < 3; i++) {
        Vector3 rotated = multiply_matrix_vector(rotMat, points[i]);
        drawPixel((int)(center.x + rotated.x), (int)(center.y + rotated.y), ch);
    }
}

void drawSquare(Vector3 center, float size, const char* ch, float angle) {
    Matrix3x3 rotMat = rotation_matrix(angle);
    Vector3 points[4] = {
        {-size, -size, 1},
        {size, -size, 1},
        {size, size, 1},
        {-size, size, 1}
    };

    for (int i = 0; i < 4; i++) {
        Vector3 rotated = multiply_matrix_vector(rotMat, points[i]);
        drawPixel((int)(center.x + rotated.x), (int)(center.y + rotated.y), ch);
    }
}

void drawCircle(Vector3 center, float radius, const char* ch) {
    for (int y = (int)(center.y - radius); y <= (int)(center.y + radius); y++) {
        for (int x = (int)(center.x - radius); x <= (int)(center.x + radius); x++) {
            if ((x - center.x) * (x - center.x) + (y - center.y) * (y - center.y) <= radius * radius) {
                drawPixel(x, y, ch);
            }
        }
    }
}

int main() {
    int gameState = 0;  // 0: 초기화면, 1: 정렬화면, 2: 애니메이션
    startTime = clock();

    while (1) {
        if (_kbhit()) {
            char key = _getch();
            if (key == 27) // ESC
                break;
            if (key == ' ') {
                gameState = (gameState + 1) % 3;
                if (gameState == 2) {
                    startTime = clock(); // 애니메이션 시작 시 시간 초기화
                }
            }
        }

        clearScreen();

        if (gameState == 0) {
            // 학번과 이름 표시
            const char* info = "20231360 김원희";
            int infoLen = strlen(info);
            for (int i = 0; i < infoLen; i++) {
                screenBuffer[CENTER_Y][(CENTER_X - infoLen / 2 + i) * 2] = info[i];
            }
        }
        else {
            Vector3 sunPos = { CENTER_X, CENTER_Y, 1 };
            Vector3 earthPos, moonPos;

            if (gameState == 1) {
                // 정렬 화면: 일직선 배치
                earthPos = (Vector3){ CENTER_X + 12, CENTER_Y, 1 };
                moonPos = (Vector3){ CENTER_X + 16, CENTER_Y, 1 };

                drawTriangle(sunPos, 5, "SS", 0);
                drawSquare(earthPos, 3, "EE", 0);
                drawCircle(moonPos, 1.5f, "MM");
            }
            else {  // gameState == 2
                elapsedTime = (float)(clock() - startTime) / CLOCKS_PER_SEC;

                // 각도 계산
                sunRotation = (elapsedTime * 120.0f);         // 3초당 1바퀴 (반시계)
                earthRotation = -(elapsedTime * 180.0f);      // 2초당 1바퀴 (시계)
                earthOrbit = (elapsedTime * 120.0f);          // 3초당 1바퀴 (반시계)
                moonRotation = -(elapsedTime * 360.0f);       // 1초당 1바퀴 (시계)
                moonOrbit = -(elapsedTime * 180.0f);          // 2초당 1바퀴 (반시계)

                // 태양 그리기
                drawTriangle(sunPos, 5, "SS", sunRotation);

                // 지구 위치 계산 및 그리기
                Matrix3x3 earthOrbitMat = multiply_matrices(
                    translation_matrix(CENTER_X, CENTER_Y),
                    multiply_matrices(
                        rotation_matrix(earthOrbit),
                        translation_matrix(12, 0)
                    )
                );
                earthPos = multiply_matrix_vector(earthOrbitMat, (Vector3) { 0, 0, 1 });
                drawSquare(earthPos, 3, "EE", earthRotation);

                // 달 위치 계산 및 그리기
                Matrix3x3 moonOrbitMat = multiply_matrices(
                    translation_matrix(earthPos.x, earthPos.y),
                    multiply_matrices(
                        rotation_matrix(moonOrbit),
                        translation_matrix(4, 0)
                    )
                );
                moonPos = multiply_matrix_vector(moonOrbitMat, (Vector3) { 0, 0, 1 });
                drawCircle(moonPos, 1.5f, "MM");
            }
        }

        renderScreen();
        Sleep(33); // 약 30 FPS
    }

    return 0;
}