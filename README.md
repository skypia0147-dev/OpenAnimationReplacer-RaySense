# Open Animation Replacer - RaySense

An SKSE plugin for Open Animation Replacer (OAR) that provides vertical elevation and obstacle detection.

## Overview
This plugin utilizes a raycasting system to detect real-time terrain height variations and obstacles around the character. The detected data is exposed as custom conditions for OAR, enabling dynamic animation transitions based on terrain elevation or the presence of obstacles.

## Key Features
- **RaySense_Verticality**: Detects terrain height differences at the front, left, right, and the character's current position.
- **RaySense_Obstacle**: Detects the distance to obstacles (walls, ledges, etc.) in front of the character.
- **Global Variable Integration**: Calculated values are synchronized with Skyrim’s Global Variables, allowing them to be referenced by external scripts.

## Requirements
- Skyrim Special Edition / Anniversary Edition
- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [Open Animation Replacer](https://www.nexusmods.com/skyrimspecialedition/mods/92109)

## Development Info
- **Language**: C++23
- **Library**: CommonLibSSE-NG
- **Build System**: CMake, vcpkg

## License
The source code of this project is free to reference and modify.




# Open Animation Replacer - RaySense

Open Animation Replacer(OAR)를 위한 수직 고도 및 장애물 감지 SKSE 플러그인입니다.

## 개요
이 플러그인은 레이캐스트(Raycast) 시스템을 사용하여 캐릭터 주변의 지형 높이 변화와 전방의 장애물을 실시간으로 감지합니다. 감지된 데이터는 OAR의 커스텀 조건(Custom Conditions)으로 노출되어, 지형의 고저차나 장애물 유무에 따른 동적인 애니메이션 전환을 구현하는 데 사용할 수 있습니다.

## 주요 기능
- **RaySense_Verticality**: 전방, 좌측, 우측 및 캐릭터 현재 위치의 지면 높이 차이를 감지합니다.
- **RaySense_Obstacle**: 캐릭터 전방의 장애물(벽, 단차 등)과의 거리를 감지합니다.
- **Global Variable 연동**: 계산된 값들은 스카이림의 Global Variable과 동기화되어 외부 스크립트에서도 참조할 수 있습니다.

## 요구 사항
- Skyrim Special Edition / Anniversary Edition
- [SKSE64](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
- [Open Animation Replacer](https://www.nexusmods.com/skyrimspecialedition/mods/92109)

## 개발 정보
- **Language**: C++23
- **Library**: CommonLibSSE-NG
- **Build System**: CMake, vcpkg

## 라이선스
이 프로젝트의 소스 코드는 자유롭게 참조 및 수정이 가능합니다.
