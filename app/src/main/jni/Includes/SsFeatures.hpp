// SsFeatures.hpp — Subway Surf 3.70.3, arm64-v8a, libil2cpp.so (81111792 байт).
// Положить в app/src/main/jni/Includes/. Подключать в Main.cpp ПОСЛЕ Macros.h и после
// #define targetLibName. Все адреса — VA (смещения от базы libil2cpp.so в памяти),
// то же, что RVA в il2cpp-дампе. Это единственное место с адресами под версию игры.
#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

// --- конфигурация под версию 3.70.3 ---
// "Не умирать": ret в начале CharacterStumble.Stumble x3, DefaultStumbleBehaviour.HandleStumble,
// CharacterStumble.Kill x2, StumbleDispatcher.DispatchKilled
static const char *const kSsNoDie[] = {"0x1F52D58", "0x1F52E7C", "0x1F52F78", "0x1F53EC8",
                                       "0x1F5318C", "0x1F53300", "0x1F53980"};
static const char *const kSsRet = "C0 03 5F D6";

// "Проход сквозь": CharacterCollision.UpdateLayers, tbz w21,#0 (95 02 00 36) -> b (14 00 00 14)
static const char *const kSsPassOff = "0x1F11FF0";
static const char *const kSsPassOn = "14 00 00 14";

// "Скорость": геттеры CharacterMotor.StartSpeed / EndSpeed возвращают константу
static const char *const kSsSpeedOff[] = {"0x1F129CC", "0x1F129E8"};

static bool gSsSpeedOn = false;
static bool gSsSpeedApplied = false;
static int gSsSpeed = 160;

// hex инструкций: movz w8,#hi,lsl16 [; movk w8,#lo] ; fmov s0,w8 ; ret
inline std::string SsSpeedHex(float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, 4);
    uint32_t hi = bits >> 16, lo = bits & 0xFFFFu;
    uint32_t ins[4];
    int n = 0;
    ins[n++] = 0x52A00008u | (hi << 5);
    if (lo) ins[n++] = 0x72800008u | (lo << 5);
    ins[n++] = 0x1E270100u;
    ins[n++] = 0xD65F03C0u;
    std::string s;
    char buf[16];
    for (int i = 0; i < n; i++) {
        std::snprintf(buf, sizeof buf, "%02X %02X %02X %02X ", ins[i] & 0xFF,
                      (ins[i] >> 8) & 0xFF, (ins[i] >> 16) & 0xFF, (ins[i] >> 24) & 0xFF);
        s += buf;
    }
    if (!s.empty()) s.pop_back();
    return s;
}

// Значение патча зашито в кэше pExpress при первом создании, поэтому для нового значения
// сначала возвращаем оригинал, потом сбрасываем кэш и создаём патч заново.
inline void SsApplySpeed() {
    for (const char *off : kSsSpeedOff) {
        if (gSsSpeedApplied) {
            DobbyPatchWrapper(targetLibName, off, "", false);
            pExpress.erase(std::string(off));
        }
    }
    gSsSpeedApplied = false;
    if (!gSsSpeedOn) return;
    for (const char *off : kSsSpeedOff)
        DobbyPatchWrapper(targetLibName, off, SsSpeedHex((float) gSsSpeed), true);
    gSsSpeedApplied = true;
}

// Номера функций = порядок в GetFeatureList (Category/ButtonLink не считаются)
inline void SsChanges(int featNum, int value, bool on) {
    switch (featNum) {
        case 0:
            for (const char *off : kSsNoDie) DobbyPatchWrapper(targetLibName, off, kSsRet, on);
            break;
        case 1:
            DobbyPatchWrapper(targetLibName, kSsPassOff, kSsPassOn, on);
            break;
        case 2:
            gSsSpeedOn = on;
            SsApplySpeed();
            break;
        case 3:
            // value приходит из поля ввода: любое целое число; 0 = ничего не вводили (игнор)
            if (value >= 1) {
                gSsSpeed = value > 100000 ? 100000 : value;
                if (gSsSpeedOn) SsApplySpeed();   // применяется сразу, в том числе во время бега
            }
            break;
        default:
            break;
    }
}
