# Open Animation Replacer - RaySense (Consolidated Edition)

**RaySense** is an advanced SKSE plugin designed to extend the capabilities of the Open Animation Replacer (OAR). It utilizes real-time Havok raycasting to detect verticality, slopes, obstacles, and surface types.

---

## Core Features

### 1. Stair & Terrain Smoothing
- Beyond simple raycasting, RaySense uses Differential Sensing to ensure smooth character movement on stairs.
- It treats stairs as a **smooth ramp** rather than individual steps, allowing animations to blend naturally.

### 2. Mid-air Velocity Prediction
- While the player is in mid-air, the plugin predicts the **height and slope of the landing spot**.
- This enables OAR to trigger landing-specific animations before touching the ground.

### 3. Dynamic Jump Bonus
- Dynamically adjusts `fJumpHeightMin` when an obstacle is detected while moving to provide an **extra jump boost**.

### 4. Smart Caching (Performance)
- Pauses heavy calculations when the player is stationary to minimize CPU impact.

---

## OAR Custom Conditions Reference

These consolidated conditions are available in the OAR UI and `config.json`.

### 1. RaySense_Verticality (Elevation)
- `Index 0` (Front): Elevation diff (80 units ahead). **(Predicts landing spot while mid-air)**
- `Index 1` (Left): Elevation diff (50 units left).
- `Index 2` (Right): Elevation diff (50 units right).
- `Index 3` (Player): Distance to solid ground.
- `Index 4` (SurfaceType): Material ID of the ground.
- `Index 5` (PlatformType): 1: Moving Platform, 2: Actor.

### 2. RaySense_Slope (Angles)
- `Index 0` (Surface): Physical surface angle.
- `Index 1` (Front): Macro terrain slope ahead. **(Supports uphill/downhill smoothly)**
- `Index 2` (Left): Macro terrain slope on left.
- `Index 3` (Right): Macro terrain slope on right.
- *Note: Steep slopes (>75°) identified as walls are automatically treated as 0.*

### 3. RaySense_Wall (Distances)
- `Index 0: Front / 1: FrontL / 2: FrontR / 3: Left / 4: Right`

### 4. RaySense_Obstacle (Details)
- `Index 0: FrontType / 1: LeftType / 2: RightType (FormID)`
- `Index 3`: Vault Distance (Parkour dist)

#### Common Obstacle Type IDs (Index 0-2)
These numeric IDs represent the type of object detected:
- `34`: Static (Most walls/rocks)
- `38`: Tree
- `40`: Furniture (Benches, beds)
- `29`: Door
- `24`: Activator (Levers, buttons)
- `36`: Movable Static
- `43`: NPC (Useful for "stumbling" animations)

### Surface Material IDs (Index 4)
When using `Index 4 (SurfaceType)`, use these values:
- `1` : Grass / `2` : Snow / `3` : Ice / `4` : Water / `5` : Wood / `6` : Stone / `7` : Dirt / `8` : Sand / `9` : Gravel

### Platform Types (Index 5)
- `0` : None (Static ground)
- `1` : Moving Platform (Elevators)
- `2` : Actor (Standing on another NPC/Creature)

---

## TESGlobal Variables

| EditorID | Description |
| :--- | :--- |
| `RaySense_SurfaceSlope` | Standable surface angle |
| `RaySense_MacroSlope_Front` / `_Left` / `_Right` | Macro terrain angles |
| `Verticality_Front` / `_Left` / `_Right` | Elevation differences |
| `RaySense_Wall_Front` / `_Left` / `_Right` | Distance to walls |
| `RaySense_SurfaceType` / `_PlatformType` | Material / Platform |
| `Obstacle_Type_Front` / `_Left` / `_Right` | FormType IDs |
| `Verticality_Obstacle` | Vault distance |

---

## Requirements

- **[SKSE64](https://skse.silverlock.org/)**
- **[Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)**
- **[Open Animation Replacer (OAR)](https://www.nexusmods.com/skyrimspecialedition/mods/92109)** (Version 3.0.2+ Required)

---

## Configuration (INI)

RaySense can be customized via `OpenAnimationReplacer-RaySense.ini`.

- **[Debug]**
  - `bEnableLog`: Enable/disable detailed sensor logging to SKSE logs.
  - `bEnableDebugDraw`: Visualize sensor detection points in-world as dots. (Default: 0)
- **[Tweaks]**
  - `fFurnitureExitCooldown`: Time (seconds) to pause sensors after exiting furniture (prevents glitchy reads).
  - `fObstacleWalkDistance`: Raycast distance for obstacle detection while walking.
  - `fObstacleSprintDistance`: Raycast distance for obstacle detection while sprinting.
  - `bEnableJumpBonus`: Enable/disable automatic jump height boost when facing an obstacle.
  - `fJumpBonus`: Amount of extra jump height added if `bEnableJumpBonus` is true.

---

## Credits
- **Ershin**: For the incredible Open Animation Replacer (OAR) and its extensible C++ API.
- **doodlum**: For the native water detection (GetSubmergedLevel) inspiration from their work on Wade in Water.

---

# 한국어 설명서 (Korean)

**RaySense**는 Open Animation Replacer (OAR)의 기능을 확장하기 위해 설계된 고급 SKSE 플러그인입니다. 실시간 Havok 레이캐스팅을 활용하여 지형의 고도, 경사, 장애물 및 표면 재질을 감지합니다.

---

UI 최적화를 위해 수십 개의 컨디션을 4가지 핵심 카테고리로 통합했습니다.

---

## 핵심 기능 (Core Features)

### 1. 지형 평활화 및 계단 감지
- 단순한 레이캐스트를 넘어 계단이나 복잡한 오브젝트 위에서도 캐릭터가 부드럽게 움직일 수 있도록 **차분 센싱** 기술을 사용합니다.
- 계단을 개별 턱이 아닌 **부드러운 경사면(Ramp)**으로 인식하여 애니메이션이 끊기지 않도록 돕습니다.

### 2. 체공 중 위치 예측
- 플레이어가 공중에 있을 때 현재 속도와 방향을 계산하여 **착지 예정 지점의 높이와 경사**를 예측합니다.
- 이를 통해 착지 직전 상황에 맞는 애니메이션을 선제적으로 출력할 수 있습니다.

### 3. 동적 점프 보너스
- 전방에 낮은 장애물이 감지된 상태에서 점프할 경우 `fJumpHeightMin` 값을 실시간으로 보정하여 **추가 점프 높이**를 제공합니다.

### 4. 스마트 캐싱 (최적화)
- 플레이어가 정지해 있을 때 불필요한 연산을 중단하여 성능 부하를 최소화합니다.

---

## OAR 커스텀 컨디션 참조 (OAR Custom Conditions Reference)

OAR UI 및 `config.json`에서 아래 컨디션들을 사용할 수 있습니다.

### 1. RaySense_Verticality (지형 고도)
- `Index 0: 전방 / Index 1: 좌측 / Index 2: 우측` **(체공 중에는 착지 지점 예측)**
- `Index 3 (Player)`: 현재 체공 높이
- `Index 4 (SurfaceType)`: 바닥 재질 번호
- `Index 5 (PlatformType)`: 움직이는 발판(1) 또는 액터(2) 위 여부

### 2. RaySense_Slope (경사도)
- `Index 0 (Surface)`: 현재 서 있는 지면의 물리 각도
- `Index 1 (Front)`: 전방 지형의 거시적 경사 (**오르막/내리막 완벽 지원**)
- `Index 2: Left` / `Index 3: Right`: 좌/우측 거시 경사
- *참고: 벽으로 판정되는 75도 이상의 급경사는 자동으로 0으로 처리됩니다.*

### 3. RaySense_Wall (벽 거리)
- `Index 0: 전방 / 1: 전좌 / 2: 전우 / 3: 좌 / 4: 우`

### 4. RaySense_Obstacle (장애물)
- `Index 0~2: 방향별 장애물 타입 (FormID)`
- `Index 3`: 볼팅 거리

#### 주요 장애물 타입 ID (Index 0-2)
감지된 객체의 종류를 나타내는 숫자 ID입니다:
- `34`: Static (대부분의 벽, 바위)
- `38`: Tree (나무)
- `40`: Furniture (가구, 벤치)
- `29`: Door (문)
- `24`: Activator (작동기)
- `36`: Movable Static (움직이는 고정물)
- `43`: NPC (충돌 애니메이션 등에 활용)

### 바닥 재질 (Surface) 번호 (Index 4)
- `1`: 풀 / `2`: 눈 / `3`: 얼음 / `4`: 물 / `5`: 나무 / `6`: 돌 / `7`: 진흙 / `8`: 모래 / `9`: 자갈

### 플랫폼 종류 (Platform) 번호 (Index 5)
- `0`: 없음 (일반 지형) / `1`: 움직이는 발판 / `2`: 액터 (다른 NPC/크리처 위)

---

## TESGlobal 변수 (TESGlobal Variables)

| EditorID | 설명 |
| :--- | :--- |
| `RaySense_SurfaceSlope` | 현재 서 있는 지면의 각도 |
| `RaySense_MacroSlope_Front` / `_Left` / `_Right` | 전방 및 좌우 거시 지형 경사도 |
| `Verticality_Front` / `_Left` / `_Right` | 전방 및 좌우 고도 차이 |
| `RaySense_Wall_Front` / `_Left` / `_Right` | 전방 및 좌우 벽까지의 거리 |
| `RaySense_SurfaceType` / `_PlatformType` | 지면 재질 및 플랫폼 유형 |
| `Obstacle_Type_Front` / `_Left` / `_Right` | 전방 및 좌우 장애물의 FormType ID |
| `Verticality_Obstacle` | 볼팅(Vault) 가능 거리 |

---

## 선행 모드 (Requirements)

- **[SKSE64](https://skse.silverlock.org/)**
- **[Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)**
- **[Open Animation Replacer (OAR)](https://www.nexusmods.com/skyrimspecialedition/mods/92109)** (3.0.2+ 필수)

---

## 설정 (INI 파일)

`OpenAnimationReplacer-RaySense.ini` 파일을 통해 플러그인의 동작을 세부적으로 조정할 수 있습니다.

- **[Debug]**
  - `bEnableLog`: SKSE 로그 파일에 상세 센서 데이터를 남길지 여부
  - `bEnableDebugDraw`: 월드 내에 센서 감지 지점을 점(Dot)으로 시각화하여 표시
- **[Tweaks]**
  - `fFurnitureExitCooldown`: 의자/침대에서 일어난 직후 센서 오작동 방지를 위한 일시 정지 시간(초)
  - `fObstacleWalkDistance`: 걷기 상태에서 장애물을 감지하는 레이캐스트 거리
  - `fObstacleSprintDistance`: 질주(Sprint) 상태에서 장애물을 감지하는 레이캐스트 거리
  - `bEnableJumpBonus`: 장애물 앞 점프 시 자동으로 높이 보너스 적용 여부
  - `fJumpBonus`: 장애물 점프 시 추가되는 점프 높이 값

---

## 크레딧 (Credits)
- **Ershin**: For the incredible Open Animation Replacer (OAR) and its extensible C++ API.
- **doodlum**: For the native water detection (GetSubmergedLevel) inspiration from their work on Wade in Water.
