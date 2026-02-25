# Open Animation Replacer - RaySense

**RaySense** is an advanced SKSE plugin designed to extend the capabilities of the Open Animation Replacer (OAR). It utilizes real-time Havok raycasting to detect the player's surrounding verticality (terrain elevation), obstacles, and floor surface types with extreme precision. It then exposes these calculations directly to OAR as highly efficient custom conditions.

## Key Features

- **Spatial Verticality Detection**: Automatically calculates the height difference between the player and the terrain directly in front, to the left, and to the right of the character up to 80 units away. 
- **Obstacle & Vaulting Sensing**: Casts horizontal rays from the player's knee and chest height to detect scalable obstacles (walls, fences, rocks) in real time. Perfect for creating jumping, climbing, or vaulting animation replacers.
- **Smart Surface Type Recognition**: Replaces generic ground conditions by injecting specific Material IDs (Grass, Snow, Stone, Dirt, Sand, Gravel) and collision layers.
- **Native Submerged Level System**: Implements native engine calls (`GetSubmergedLevel`) to flawlessly detect when the player wades into water, even if it's a shallow puddle or a floating ice chunk.
- **Micro-movement Caching**: Built with a "Smart Caching" system that completely bypasses heavy physics raycasts if the player is standing still, ensuring **zero FPS drop** in standard gameplay.
- **Havok Thread Safe**: Fully synchronized with Skyrim's physics engine lock (`BSReadLockGuard`) to prevent CTDs during heavy simulation scenes or teleportation.

---

## OAR Custom Conditions Reference

Once installed, you can use these custom conditions in your OAR `config.json` files or through the OAR in-game UI.

### 1. RaySense_Verticality

Measures the elevation difference between the target terrain and the player's current Z-position.

**Syntax**: `RaySense_Verticality [Sensor] [Comparison] [Value]`

**Sensors**:
- `Front` (0): Terrain elevation up to 80 units directly ahead.
- `Left` (1): Terrain elevation up to 50 units directly left.
- `Right` (2): Terrain elevation up to 50 units directly right.
- `Player` (3): Distance between the player's root bone and the solid ground below them (useful for predicting landing while in mid-air).
- `Surface` (4): Returns the Material ID enum of the ground the player is standing on (See *Surface Material IDs* below).
- `Platform` (5): Returns whether the player is standing on a moving platform (1) or another actor (2).

**Example**:
- `RaySense_Verticality Front > 30` : True if the terrain 80 units in front of the player is more than 30 units higher than the player's current position (e.g., walking uphill or facing stairs).

### 2. RaySense_Obstacle

Measures distance to an obstacle that blocks the player's knees but NOT the player's chest (Vaultable obstacles).
- **Example**: `RaySense_Obstacle < 60` : True if there is a low wall or fence within 60 units.

### 3. RaySense_Wall_[Direction]

Measures the absolute distance to a solid object in a specific direction.
- `RaySense_Wall_Front`: Distance ahead.
- `RaySense_Wall_Front_L`: Distance to the front-left.
- `RaySense_Wall_Front_R`: Distance to the front-right.
- `RaySense_Wall_Left`: Distance directly left.
- `RaySense_Wall_Right`: Distance directly right.

**Example**:
- `RaySense_Wall_Right < 40` : True if a wall or solid object is very close on the right side. Great for triggering hand-on-wall animations.

---

## Surface Material IDs (Verticality Sensor: 4)

When using `RaySense_Verticality Surface == [Value]`, use the following integer values:

- `1` : Grass
- `2` : Snow
- `3` : Ice
- `4` : Water (Also triggers dynamically based on submersion depth > 0.05)
- `5` : Wood (Props, Trees, Bridges, Barrels)
- `6` : Stone
- `7` : Dirt / Mud
- `8` : Sand
- `9` : Gravel

### Platform Types (Sensor 5: Platform)

When using `RaySense_Verticality Platform == [Value]`, you can detect moving objects under the player:

- `0` : None (Standard static ground or terrain)
- `1` : Moving Platform (Elevators, moving structures)
- `2` : Actor (Standing on top of another actor, e.g., a Dragon)

---
## Performance Note
This plugin is heavily optimized by a Senior SKSE developer. It employs internal `std::atomic` caches and early exits (such as skipping operations during swimming, mounting, or killmoves) to minimize Havok polling. Feel free to use these conditions liberally in your OAR setups.

---
## Requirements

- **[SKSE64](https://skse.silverlock.org/)**
- **[Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)**
- **[Open Animation Replacer (OAR)](https://www.nexusmods.com/skyrimspecialedition/mods/92109)** (Version 2.3.6+ or 3.0+ Recommended)

## Credits

- **Smooth**: Author & Creator of RaySense.
- **Ershin**: For the incredible Open Animation Replacer (OAR) and its extensible C++ API.
- **doodlum**: For the `GetSubmergedLevel` native engine code reference from `Wade in Water`, crucial for the accurate water detection logic.

---

# 한국어 설명서 (Korean)

**RaySense**는 실시간 Havok 물리 레이캐스트를 활용하여 플레이어 주변의 지형 고저차(Verticality), 장애물, 바닥 재질을 극히 정밀하게 감지하고 이를 OAR 조건식으로 사용할 수 있게 해주는 고도의 SKSE 플러그인입니다.

## 주요 기능
- **지형 고저차 감지**: 플레이어 전방, 좌, 우 스텝 위치의 실제 지형 높이 차이를 계산하여 오르막/내리막/계단 전용 모션을 구동할 수 있습니다.
- **장애물 및 파쿠르 인식**: 가슴 높이는 뚫려있고 무릎 높이는 막혀있는 '넘어갈 수 있는(Vaultable)' 장애물까지의 거리를 반환합니다. 파쿠르 및 점프 모션 제작에 필수적입니다.
- **스마트 재질 인식**: 눈, 풀, 얼음, 돌 모양에 따른 바닥 재질을 감지하며, 특히 얕은 물웅덩이나 발목 높이 이상의 물을 엔진 네이티브(`GetSubmergedLevel`)로 파악해 완벽한 물보라 모션 전환이 가능합니다.
- **극강의 프레임(FPS) 방어**: 제자리에 서 있을 때 물리 연산을 즉시 생략하는 마이크로 캐싱이 도입되어 있어 모드 설치로 인한 프레임 드랍이 전혀 없습니다. 멀티스레드 락(Lock) 동기화로 CTD 또한 원천 차단됩니다.

## OAR 조건식 설명 (간단 요약)

### 1. RaySense_Verticality (지형 고도)
`RaySense_Verticality [센서위치] [비교] [값]`
- **센서 위치 (숫자 0~5 입력 가능)**
  - `Front (0)`: 전방 지형의 고도차
  - `Left (1)`: 좌측 지형의 고도차
  - `Right (2)`: 우측 지형의 고도차
  - `Player (3)`: 공중에 떠 있을 때, 땅에 닿기까지 남은 높이 (착지 모션용)
  - `Surface (4)`: 현재 밟고 있는 바닥 재질 (아래 번호 표 참조)
  - `Platform (5)`: 움직이는 다리(1)나 다른 액터(2) 위에 서 있는지 여부

### 2. RaySense_Obstacle (파쿠르용 장애물)
`RaySense_Obstacle < [거리]`
- 가슴 높이는 비어있고 무릎은 막힌 파쿠르 가능한 장애물까지의 거리를 계산합니다.

### 3. RaySense_Wall_[방향] (벽 감지)
`RaySense_Wall_Front`, `RaySense_Wall_Left` 등 방향별로 조밀한 벽이나 오브젝트까지의 거리를 측정합니다. 손으로 벽을 짚는 애니메이션 등에 유용합니다.

### 바닥 재질 (Surface) 번호 목록 (센서 위치 4번 세팅 시)
- `1` : 풀 (Grass)
- `2` : 눈 (Snow)
- `3` : 얼음 (Ice)
- `4` : 물 (Water - 발목 이상 잠기면 레이캐스트와 무관하게 즉시 발동)
- `5` : 나무 (Wood - 통나무, 나무판자, 가구 등)
- `6` : 돌 (Stone)
- `7` : 진흙/흙 (Dirt)
- `8` : 모래 (Sand)
- `9` : 자갈 (Gravel)

### 플랫폼 종류 (Platform Type) 번호 목록 (센서 위치 5번 세팅 시)
`RaySense_Verticality Platform == [Value]` 형태로 사용을 권장합니다.
- `0` : None (일반 바닥, 고정된 지형)
- `1` : Moving Platform (움직이는 발판, 엘리베이터 형태의 구조물)
- `2` : Actor (드래곤, 거인 등 다른 액터 위에 밟고 서 있는 상태)

---
## 선행 모드 (Requirements)

- **[SKSE64](https://skse.silverlock.org/)**
- **[Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)**
- **[Open Animation Replacer (OAR)](https://www.nexusmods.com/skyrimspecialedition/mods/92109)**

## 크레딧 (Credits)

- **Smooth**: 플러그인 제작 및 로직 설계
- **Ershin**: Open Animation Replacer (OAR) 원작자 및 조건식 API 제공
- **doodlum**: 정확한 수위 판정에 큰 도움이 된 `Wade in Water` 플러그인의 네이티브 엔진 코드 레퍼런스 제공
