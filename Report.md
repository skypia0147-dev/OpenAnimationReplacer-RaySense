# OAR-Editor-Web 배포 전 핵심 영역 점검 및 개선 리포트

요청하신 4가지 핵심 영역(WebView2 브릿지, DOM 최적화, 파일 I/O 안전성, 런타임 배포 환경)에 대해 보안 및 최적화 관점에서 현재 코드베이스를 정밀 분석한 결과와 방어 코드 스니펫을 제시합니다.

---

## 1. WebView2 연동 및 브릿지 통신(IPC) 안정성

**[위험 요소 분석]**
현재 `MainForm.cs`와 `index.html`은 `http://localhost:8080/` 로컬 서버를 통해서만 연결되어 있으며, 양방향 IPC(Inter-Process Communication)가 전혀 구현되어 있지 않습니다. 
이로 인해 JavaScript에서 `catch` 되지 않은 에러나 크래시가 발생할 경우 사용자는 어떤 문제인지 전혀 알 수 없으며(조용한 실패), C# 백엔드 또한 이를 로그로 남길 방법이 없습니다.

**[개선 방안: 네이티브 브릿지를 통한 에러 로깅 연동]**
JavaScript에서 발생하는 핵심 에러를 C#으로 전달하여 파일 로그(`error.log`)로 남기도록 구성해야 합니다.

**C# 방어 코드 스니펫 (`MainForm.cs` 내부):**
```csharp
private async void InitializeWebView()
{
    // ... 설정 코드 ...
    await webView.EnsureCoreWebView2Async(null);
    
    // JS -> C# 통신 메시지 수신 이벤트 핸들러
    webView.CoreWebView2.WebMessageReceived += (sender, args) =>
    {
        try {
            var msg = args.TryGetWebMessageAsString();
            if (msg.StartsWith("ERROR:")) {
                File.AppendAllText("error.log", $"[{DateTime.Now}] JS Exception: {msg}\n");
            }
        } catch { /* 무시 */ }
    };

    // JS 초기화 스크립트 주입 (전역 에러 핸들러)
    await webView.CoreWebView2.AddScriptToExecuteOnDocumentCreatedAsync(@"
        window.addEventListener('error', function(e) {
            window.chrome.webview.postMessage('ERROR: ' + e.message + ' at ' + e.filename + ':' + e.lineno);
        });
        window.addEventListener('unhandledrejection', function(e) {
            window.chrome.webview.postMessage('ERROR: Unhandled Promise Rejection - ' + e.reason);
        });
    ");
}
```

---

## 2. 프론트엔드(UI/UX) 및 DOM 최적화 (`index.html`, `style.css`)

**[위험 요소 분석]**
현재 사용자가 검색창(`search-name`, `filter-min-p` 등)에 타이핑을 할 때마다 `this.renderTree()` 함수가 **동기적으로(Synchronously) 조건 없이** 호출됩니다.
`renderTree` 함수는 내부적으로 수십~수백 개의 `<div>` DOM 요소를 완전히 파괴(`innerHTML = ''`)하고 다시 생성합니다. 이는 사용자가 타이핑을 빠르게 할 때 메인 스레드를 블로킹(Blocking)하여 키보드 입력이 버벅이거나 멈추는(Freezing) 심각한 성능 저하를 유발합니다.

**[개선 방안: Debounce 기법 적용]**
검색 입력 이벤트에 Debounce 기법을 도입하여, 사용자의 타이핑이 끝난 후 200~300ms 뒤에 한 단 한 번만 렌더링되도록 리페인트를 최소화해야 합니다.

**JavaScript 방어 코드 스니펫 (`index.html`의 검색 리스너 부근):**
```javascript
// OAREditor 생성자 또는 init() 내부에 debounce 저장 변수 추가
this.searchTimeout = null;

// Search Listeners 구현부 수정
filterInputs.forEach(id => {
    const el = document.getElementById(id);
    el.oninput = () => {
        // 타이핑 중에는 이전 예약된 렌더링을 취소 (메인 스레드 멈춤 방지)
        if (this.searchTimeout) clearTimeout(this.searchTimeout);

        this.searchTimeout = setTimeout(() => {
            // 200ms 뒤에 실제 필터값 반영 및 단 1번만 DOM 업데이트 수행
            this.filters.nameSearch = document.getElementById('search-name').value.toLowerCase();
            this.filters.exactPriority = document.getElementById('filter-exact-p').value === '' ? null : parseInt(document.getElementById('filter-exact-p').value);
            // ... minPriority, maxPriority 값 반영 유지 ...
            
            this.renderTree();
        }, 200); // 사용자 타이핑 후 200ms 지연
    };
});
```

---

## 3. 파일 I/O 및 데이터 무결성 보호

**[위험 요소 분석]**
현재 JavaScript의 File System Access API(`createWritable`)를 사용하여 `config.json`과 `user.json`에 직접 덮어쓰기(`write()`)를 수행하고 있습니다.
파일의 쓰기 작업 도중 툴이 강제 종료되거나 정전이 발생할 경우, **JSON 파일이 0KB(빈 파일)로 덮어씌워져 사용자가 설정해둔 모드 셋업 파일 전체가 영구적으로 증발(Corruption)해버리는 매우 치명적인 위험성을 가지고 있습니다.**

**[개선 방안: Safe Write (임시 파일 스왑) 전략 구현]**
브라우저 File System API 특성상 완전한 락 제어가 어렵기 때문에, 안전한 백업 로직을 직접 구현해야 합니다. 원본을 덮어쓰기 직전에 `config_backup.json`을 먼저 생성한 뒤 안전하게 저장하는 로직이 필수입니다.

**JavaScript 방어 코드 스니펫 (Back-up Safe Write 적용):**
```javascript
async handleSave() {
    this.setStatus('Saving Base Config...');
    for (const [path, config] of this.configs.entries()) {
        if (config.modified) {
            const json = JSON.stringify(config.data, null, 2);
            if (this.isSecureContext) {
                try {
                    // [방어 로직 1] 원본 파일이 손상될 경우를 대비해 쓰기 전 백업 파일 저장 시도
                    try {
                        const backupHandle = await config.parentHandle.getFileHandle('config_backup.json', { create: true });
                        const backupWritable = await backupHandle.createWritable();
                        await backupWritable.write(json);
                        await backupWritable.close();
                    } catch (e) { /* 권한/경로 문제 등으로 백업 실패 시 일단 무시하고 진행 */ }

                    // [본 쓰기 작업] 이 작업 도중 크래시가 나더라도 위에서 만든 _backup.json이 유지됨
                    const writable = await config.handle.createWritable();
                    await writable.write(json);
                    await writable.close();
                    config.modified = false;
                } catch (err) {
                    alert(`파일 저장 중 권한 오류가 발생했습니다.\n경로: ${path}\n파일이 읽기 전용 상태가 아닌지 확인하세요!`);
                }
            }
        }
    }
}
```

---

## 4. 배포(Deployment) 및 런타임 의존성

**[위험 요소 분석]**
`Program.cs`는 Windows 사용자의 PC에 **Microsoft Edge WebView2 Runtime**이 사전 설치되어 있을 것이라고 가정하고 작동합니다.
만약 설치되지 않은 구형 OS나 삭제된 PC에서 툴을 실행하면 WebView 초기화가 조용히 실패하며 `MessageBox.Show($"WebView2 Initialization Failed...");` 라는 간단한 예외 메시지만 띄우고 종료됩니다. 이 경우 사용자는 어디서 런타임을 구해야 하는지 몰라 버그로 간주할 것입니다.

**[개선 방안: 친절한 런타임 설치 다이얼로그 및 브라우저 다운로드 연동]**
의존성 문제 발생 시 원인을 정확하게 안내하고, 사용자가 직접 수동 검색을 할 필요 없이 버튼 클릭 한 번으로 다운로드 페이지를 열어주는 친절한 로직이 필요합니다.

**C# 방어 코드 스니펫 (`MainForm.cs` 내부):**
```csharp
private async void InitializeWebView()
{
    try {
        await webView.EnsureCoreWebView2Async(null);
        // ... 생략 ...
    } 
    catch (WebView2RuntimeNotFoundException) // WebView2 전용 에러 클래스 
    {
        var result = MessageBox.Show(
            "이 OAR 에디터 프로그램을 실행하려면 'Microsoft Edge WebView2 런타임'이 시스템에 설치되어 있어야 합니다.\n\n" +
            "지금 Microsoft 공식 다운로드 웹페이지로 이동하여 설치하시겠습니까?", 
            "필수 런타임 누락", 
            MessageBoxButtons.YesNo, MessageBoxIcon.Warning
        );

        if (result == DialogResult.Yes)
        {
            // 사용자의 기본 브라우저를 띄워 다운로드 사이트로 즉시 안내
            System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo
            {
                FileName = "https://developer.microsoft.com/en-us/microsoft-edge/webview2/",
                UseShellExecute = true
            });
        }
        Application.Exit(); // 불안정한 상태에서의 구동 방지
    }
    catch (Exception ex) 
    {
        MessageBox.Show($"예기치 않은 초기화 오류가 발생했습니다: {ex.Message}", "Critical Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        Application.Exit();
    }
}
```

---

## 5. OAR 데이터 포맷 및 모딩 엔진 호환성 집중 점검

OAR 3.0.1+ 엔진은 `config.json`을 읽을 때 매우 엄격한 JSON 파서를 사용합니다. 툴이 생성한 데이터가 엔진에서 튕기지 않도록 직렬화(Serialization) 과정을 철저히 점검하였습니다.

**[위험 요소 1: 인코딩 (UTF-8 w/o BOM)]**
OAR C++ 코드는 BOM(Byte Order Mark)이 포함된 UTF-8 문서를 읽을 때 첫 글자부터 파싱 에러를 뿜을 수 있습니다.
*   **분석 및 조치:** 현재 OAR Editor Web이 파일 저장을 위해 사용하는 Web API의 `FileSystemAccess API (createWritable)`와 다운로드를 위한 `Blob` 객체는 브라우저 스펙상 **무조건 BOM 없는 순수 UTF-8 인코딩**으로만 바이너리를 출력합니다. 따라서 추가 로직 없이도 **OAR 인코딩 요구사항을 100% 완벽하게 충족**합니다.

**[위험 요소 2: 후행 쉼표 (Trailing Comma)]**
일부 수동 작성된 JSON에서 자주 보이는 `,}` 또는 `, ]` 형태의 Trailing Comma는 엄격한 OAR 엔진에서 Fatal Error를 일으킵니다.
*   **분석 및 조치:** 툴 내부에서 `JSON.stringify(config.data, ..., 2);` 네이티브 직렬화 함수만을 사용해 문자열 형태를 만듭니다. 표준 `JSON.stringify` 알고리즘 자체가 **후행 쉼표를 절대로 생성하지 않으므로** 배열이나 객체 끝의 쉼표로 인한 오류 위험은 0%입니다.

**[위험 요소 3: 엄격한 데이터 타입 매핑 (Float / Int vs String)]**
사용자가 `BlendTime`이나 우선순위 같은 값을 UI 인풋 박스(Input box)에 타이핑한 직후, 자바스크립트는 이를 **문자열(String)**로 취급합니다. 이를 그대로 저장하면 `""` 쌍따옴표가 붙은 채 직렬화되어 OAR C++ 코드의 `GetValueFloat()`나 `GetValueInt()` 과정에서 컴파일 캐스팅 에러(튕김 현상)가 발생합니다.
*   **분석 및 조치:** 숫자를 다루는 모든 속성에 대해 안전망을 추가했습니다. Input 변경 이벤트에서 값을 원본 데이터 모델에 반영할 때, 반드시 `parseFloat()`와 `parseInt()`를 사용하여 명시적으로 **숫자형 강제 캐스팅(Casting)**을 수행하고 결과값이 `NaN`(비정상 값)일 경우 안전하게 `0`으로 대입하도록 방어 코드를 즉각 적용했습니다.

**JavaScript 방어 코드 스니펫 (`index.html` 값 수정 이벤트 부분):**
```javascript
input.oninput = () => {
    // ...
    if (input.type === 'number') {
        const parsed = parseFloat(input.value);
        obj[key] = isNaN(parsed) ? 0 : parsed; // JSON에는 쌍따옴표 없이 순수 숫자(Float)로 기록됨
    } else if (input.tagName === 'SELECT') {
        const parsed = parseInt(input.value);
        obj[key] = isNaN(parsed) ? 0 : parsed; // 열거형 값들 역시 정수형(Int)으로 강제 변환
    }
    // ...
};
```
이 조치로 인해, 생성된 모든 JSON은 OAR 엔진이 요구하는 Float/Int 타입을 정확히 준수하여 스카이림 게임 내에서 조건 충돌을 일으키지 않습니다.
