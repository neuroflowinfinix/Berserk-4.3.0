Berserk 4.3.0 Chess Engine compiled to WebAssembly. Sub-7MB client-side engine for browser-based chess analysis. Pure JavaScript execution, offline capable, UCI protocol support. Lightweight alternative to Stockfish for web applications. 
        
              
        # Berserk WASM

        WebAssembly port of [Berserk 4.3.0](https://github.com/jhonnold/berserk) — the final HCE release.

        ## Structure
        | Path | Contents |
        |------|----------|
        | `src/` | Compiled `berserk.wasm`, Emscripten glue `berserk.js`, Web Worker |
        | `source/` | Original C source, pre-patched for WASM |
        | `example/` | Functional HTML test UI |
        | `build.sh` | Rebuild script |

        ## Quick Start
```bash
        cd example && python3 -m http.server 8080
        # open http://localhost:8080
```

        ## API
```javascript
        const engine = new Worker('src/worker.js');
        engine.onmessage = (e) => console.log(e.data);
        engine.postMessage('uci');
        engine.postMessage('position startpos moves e2e4');
        engine.postMessage('go depth 15');
```
