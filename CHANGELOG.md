# Changelog

## 2026-01-04
- Removed msgpack-c and lv_drivers dependencies; switched manifests to header-only nlohmann/json.
- Fixed cross-compile issues (logging macro conflicts, LVGL NEON ASM disabled, POSIX FS letter/path, textarea enable).
- Cleaned CMake linkage (touchdown-core target name, dropped lv_drivers links) and added nlohmann include path.
- Updated Settings and System Info apps to register correctly with LVGL v9 APIs and app registry.
- Stabilized services (power/input) and driver includes; added DRM fourcc include.
- Restored circular masking for the round display via LVGL clip-corner and radius styling.
- Simplified shell input handling and ensured launcher/app container masking; build now completes for shell/services/apps.
