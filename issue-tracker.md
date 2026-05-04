# Arduino UNO Setup Issue Tracker

## 1. Missing Arduino SDK / core packages
- **Symptom:** IntelliSense reported `Serial`, `pinMode`, `digitalWrite`, `LED_BUILTIN`, `HIGH`, `LOW`, and `OUTPUT` as undefined.
- **Root cause:** The Arduino AVR core and toolchain were not installed yet.
- **Fix:** Installed `arduino-cli` and the UNO core with:
  - `arduino-cli core update-index`
  - `arduino-cli core install arduino:avr`

## 2. Board Manager command not visible in VS Code
- **Symptom:** `Arduino: Board Manager` did not appear in the Command Palette.
- **Root cause:** Extension command availability / activation was inconsistent.
- **Fix:** Used the terminal-based `arduino-cli` path as a reliable fallback.

## 3. Missing sketchbook libraries directory
- **Symptom:** IntelliSense warned that `/Users/boqi/Documents/Arduino/libraries/**` was not found.
- **Root cause:** That is the default sketchbook libraries folder, not the core install path, and it did not exist yet.
- **Fix:** Created `/Users/boqi/Documents/Arduino/libraries`.

## 4. Extension tried to launch legacy Arduino.app
- **Symptom:** Build/analyze failed with an error trying to spawn `Arduino.app/Contents/MacOS/Arduino`.
- **Root cause:** The extension was resolving to the legacy Arduino IDE executable instead of CLI mode.
- **Fix:** Forced Arduino CLI mode in `.vscode/settings.json` and pointed it at the Homebrew CLI:
  - `arduino.useArduinoCli = true`
  - `arduino.path = /opt/homebrew/bin`
  - `arduino.commandPath = arduino-cli`

## 5. IntelliSense generation failed with `code:null`
- **Symptom:** The Arduino output showed `Failed to generate IntelliSense configuration` with a generic `code:null` error.
- **Root cause:** The analyzer was failing before it could produce a useful build log, likely due to path/config resolution problems.
- **Fix:** Standardized the workspace Arduino settings and made the CLI executable path explicit.

## 6. False undefined symbols in `.ino` IntelliSense
- **Symptom:** The sketch compiled, but the editor still showed false errors for Arduino symbols.
- **Root cause:** The active C/C++ IntelliSense config was missing `Arduino.h` forced include and did not treat `.ino` as C++.
- **Fix:** Updated `.vscode/c_cpp_properties.json` to:
  - add a forced include for `Arduino.h`
  - use concrete AVR include paths
  - define UNO-specific macros
  And updated `.vscode/settings.json` to map `*.ino` to `cpp`.

## 7. Result
- **Compile:** Works.
- **IntelliSense:** Works.
- **VS Code Arduino workflow:** Stable enough to use for Arduino UNO development.

## Key takeaways
- Compile success and IntelliSense success are separate systems.
- On macOS, explicit CLI paths are often more reliable than auto-detection.
- For `.ino` files, `Arduino.h` forced include and C++ file association are essential for clean IntelliSense.
- `arduino-cli` is the best fallback when the VS Code extension UI is incomplete or flaky.
