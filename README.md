# NexCore Android (Kotlin + C++ core)

This is a minimal Android app that embeds a **C++ governance core** via JNI.

## Buttons
- **Propose**: produces a proposal JSON + audit entry.
- **Execute**: runs the proposal (draft / summary / reversible file write) with receipts + audit hash chaining.
- **Speak**: reads the output aloud via Android TextToSpeech.

## Data locations (app sandbox)
- `filesDir/nex_runtime/workspace/`
- `filesDir/nex_runtime/audit/log.jsonl`
- `filesDir/nex_runtime/snapshots/`
- `filesDir/nex_runtime/state/proposals.jsonl`

## Build
Open in Android Studio OR generate a Gradle wrapper and build with:
`./gradlew :app:assembleDebug`
