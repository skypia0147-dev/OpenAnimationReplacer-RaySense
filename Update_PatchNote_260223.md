# OAR-Editor-Web 금일 패치 로그 (2026.02.23)

## 🌟 신규 기능 (Features)
1. **EXACT PRIORITY (우선순위 정확도 검색) 필터 추가**
   - 기존의 Min/Max 방식 외에 특정 우선순위 번호만 콕 집어서 필터링할 수 있는 독립된 검색창 추가.
2. **미구현 51개 조건문(Condition) 완벽 이식**
   - OAR 최신 버전(3.0.1+)의 Source Code 구조를 분석하여 `IsSwimming`, `FallDistance` 등 UI에 없었던 51개의 핵심 조건 트리거들을 모두 선택 및 값 제어(Float/Int/Flag)가 가능하도록 추가.
3. **고급 설정(Advanced Settings) UI 탭 추가**
   - `replaceOnLoop`, `blendTimeOnEcho`, `overrideAnimationsFolder` 등 일반 유저들은 접근하기 힘들었던 `config.json`의 고급 플래그 컨트롤 옵션들을 추가.

## 🛠 안정성 최적화 및 버그 수정 (Fixes & Optimizations)
1. **[렌더링] EXACT PRIORITY NaN 에러 크래시 수정**
   - 검색창을 지웠을 때 `NaN` 값으로 인해 트리가 완전히 증발하며 멈추던 치명적인 UI 버그 픽스.
2. **[렌더링] 검색 DOM Debounce 최적화 (프리징 완화)**
   - 글자를 1글자 칠 때마다 수백~수천 개의 DOM 노드를 동기적으로 파괴하고 재구성하던 문제를 해결하여, 사용자 타이핑 완료 후 200ms 뒤에 단 1번만 재렌더링하도록 성능 구조 개편.
3. **[시스템] 세이브 파일 손실(Corruption) 방지 백업 도입**
   - 기존에는 `createWritable()`로 JSON 원본에 직접 덮어쓰기를 하행하여, 중간에 전원이 끊기거나 디스크 공간이 부족하면 기존 설정 파일마저 0KB 공백으로 증발하는 문제가 있었습니다.
   - 오늘 패치로 세이브 직전 무조건 `config_backup.json`부터 만들고 덮어쓰도록 **Safe-Write 구조**로 진화했습니다.
4. **[시스템] OAR 3.0.1+ 엔진 호환성 (데이터 타입 캐스팅)**
   - 자바스크립트는 Input 상자의 숫자를 String(문자열, 예: `"10.5"`)로 다루는 경향이 있어 OAR 엔진과 충돌할 여지가 있었습니다. 
   - 사용자가 숫자를 변경할 때 즉시 `parseFloat()`, `parseInt()`를 사용하여 저장소를 완벽히 Number 타입으로 강제 변환하도록 규격(Schema) 수정 완료.
5. **[시스템] WebView2 필수 런타임 친화적 안내 도입**
   - WebView2 런타임이 없는 타 PC에서 `Initialize Failed` 팝업만 뜨고 조용히 종료되던 문제를 보완, 런타임 다운로드 알림창과 즉시 마이크로소프트 엣지 브라우저로 리다이렉트 시키도록 `MainForm.cs` 방어 코드 삽입.
6. **[빌드] 불필요한 테스트 파일 퍼지 및 최신화**
   - 꼬여있던 `test.js`, `test.hta`를 모두 제거하고 최신의 `net9.0-windows` C# 타겟으로 Rebuild 및 `OAR-Editor-Web-Release.zip` 갱신 완료.
7. **[UI/UX] Edit Raw JSON 줄 번호 어긋남 문제 (CSS 픽스)**
   - 스크롤 시 JSON 텍스트와 좌측 숫자 넘버링의 높이가 미세하게 틀어지던 현상을, 두 영역의 `font-size`(0.95rem)와 `line-height`(1.6)를 픽셀 단위로 완벽하게 동일화하여 해결. 
8. **[UI/UX] 최상위 루트 폴더(Mod 대표 폴더)의 잘못된 옵션 표시 버그 픽스**
   - 애니메이션이 없는 모드 최상단 폴더(`config.json`)를 클릭했을 때 필요 없는 `Interruptible`, `Advanced Settings`, `Animation Conditions` 항목이 보이던 현상을 구조적으로 조건부 렌더링(Hidden 처리) 되도록 수정 완료.
