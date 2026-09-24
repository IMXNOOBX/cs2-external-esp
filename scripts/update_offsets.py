import json
import re
import sys
import urllib.request

CLIENT_DLL_URL = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/client_dll.json"
INFO_URL = "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/info.json"
OFFSETS_PATH = "src/core/offsets/Offsets.hpp"
DUMPER_PATH = "src/core/offsets/Dumper.cpp"

FIELD_MAP = {
    ("controller", "m_iPing"): "CCSPlayerController",
    ("controller", "m_hPawn"): "CCSPlayerController",
    ("controller", "m_steamID"): "CCSPlayerController",
    ("controller", "m_iszPlayerName"): "CCSPlayerController",
    ("controller", "m_bIsLocalPlayerController"): "CCSPlayerController",
    ("controller", "m_pInGameMoneyServices"): "CCSPlayerController",
    ("controller", "m_iAccount"): "CCSPlayerController_InGameMoneyServices",
    ("pawn", "m_vOldOrigin"): "C_BasePlayerPawn",
    ("pawn", "m_iHealth"): "C_BaseEntity",
    ("pawn", "m_iTeamNum"): "C_BaseEntity",
    ("pawn", "m_bIsScoped"): "C_CSPlayerPawn",
    ("pawn", "m_ArmorValue"): "C_CSPlayerPawn",
    ("pawn", "m_bIsDefusing"): "C_CSPlayerPawn",
    ("pawn", "m_vecAbsVelocity"): "C_BaseEntity",
    ("pawn", "m_pGameSceneNode"): "C_BaseEntity",
    ("pawn", "m_entitySpottedState"): "C_CSPlayerPawn",
    ("pawn", "m_flFlashOverlayAlpha"): "C_CSPlayerPawn",
    ("pawn", "m_flFlashMaxAlpha"): "C_CSPlayerPawn",
    ("pawn", "m_flFlashDuration"): "C_CSPlayerPawn",
    ("pawn", "m_iShotsFired"): "C_CSPlayerPawn",
    ("pawn", "m_pAimPunchServices"): "C_CSPlayerPawn",
    ("pawn", "m_predictableBaseAngle"): "CCSPlayer_AimPunchServices",
    ("pawn", "m_unpredictableBaseAngle"): "CCSPlayer_AimPunchServices",
    ("pawn", "m_angEyeAngles"): "C_CSPlayerPawn",
    ("pawn", "m_iIDEntIndex"): "C_CSPlayerPawn",
    ("pawn", "m_pWeaponServices"): "C_BasePlayerPawn",
    ("pawn", "m_hActiveWeapon"): "CPlayer_WeaponServices",
    ("pawn", "m_AttributeManager"): "C_EconEntity",
    ("pawn", "m_Item"): "C_AttributeContainer",
    ("pawn", "m_iItemDefinitionIndex"): "C_EconItemView",
    ("pawn", "m_iClip1"): "C_BasePlayerWeapon",
    ("pawn", "m_bInReload"): "C_CSWeaponBase",
    ("pawn", "m_pObserverServices"): "C_BasePlayerPawn",
    ("bomb", "m_bC4Activated"): "C_PlantedC4",
    ("bomb", "m_nBombSite"): "C_PlantedC4",
    ("bomb", "m_vecAbsOrigin"): "CGameSceneNode",
    ("bone", "m_modelState"): "CSkeletonInstance",
    ("observerServices", "m_iObserverMode"): "CPlayer_ObserverServices",
    ("observerServices", "m_hObserverTarget"): "CPlayer_ObserverServices",
}

def fetch_classes():
    with urllib.request.urlopen(CLIENT_DLL_URL, timeout=15) as resp:
        data = json.loads(resp.read().decode("utf-8"))
    return data["client.dll"]["classes"]

def fetch_build_number():
    with urllib.request.urlopen(INFO_URL, timeout=15) as resp:
        data = json.loads(resp.read().decode("utf-8"))
    return data["build_number"]

def resolve_offset(classes, class_name, field_name):
    seen = set()
    current = class_name
    while current and current not in seen:
        seen.add(current)
        entry = classes.get(current)
        if entry is None:
            return None
        if field_name in entry["fields"]:
            return entry["fields"][field_name]
        current = entry.get("parent")
    return None

def update_offsets_file(classes, build_number):
    with open(OFFSETS_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    namespace_stack = []
    line_pattern = re.compile(
        r'^(\s*constexpr\s+std::ptrdiff_t\s+)(\w+)(\s*=\s*)0x([0-9A-Fa-f]+)(\s*;.*)$'
    )
    namespace_open_pattern = re.compile(r'^\s*namespace\s+(\w+)\s*\{')
    build_pattern = re.compile(
        r'^(\s*constexpr\s+int\s+dumpedBuildNumber\s*=\s*)(\d+)(\s*;.*)$'
    )

    changed = []
    unresolved = []
    build_changed = None

    for i, line in enumerate(lines):
        b = build_pattern.match(line)
        if b:
            old_build = int(b.group(2))
            if old_build != build_number:
                build_changed = (old_build, build_number)
                lines[i] = f"{b.group(1)}{build_number}{b.group(3)}\n"
            continue

        ns_match = namespace_open_pattern.match(line)
        if ns_match:
            namespace_stack.append(ns_match.group(1))
            continue

        if namespace_stack and re.match(r'^\s*\}', line):
            namespace_stack.pop()
            continue

        m = line_pattern.match(line)
        if not m or not namespace_stack:
            continue

        namespace = namespace_stack[-1]
        field_name = m.group(2)
        key = (namespace, field_name)

        if key not in FIELD_MAP:
            continue

        class_name = FIELD_MAP[key]
        new_value = resolve_offset(classes, class_name, field_name)

        if new_value is None:
            unresolved.append((namespace, field_name, class_name))
            continue

        old_value = int(m.group(4), 16)
        if new_value != old_value:
            changed.append((namespace, field_name, old_value, new_value))
            lines[i] = f"{m.group(1)}{field_name}{m.group(3)}0x{new_value:X}{m.group(5)}\n"

    with open(OFFSETS_PATH, "w", encoding="utf-8") as f:
        f.writelines(lines)

    return changed, unresolved, build_changed

def main():
    classes = fetch_classes()
    build_number = fetch_build_number()
    changed, unresolved, build_changed = update_offsets_file(classes, build_number)

    if build_changed:
        print(f"Build number {build_changed[0]} -> {build_changed[1]}")
    else:
        print(f"Build number unchanged ({build_number}).")

    if changed:
        print(f"Updated {len(changed)} offset(s):")
        for namespace, field_name, old_value, new_value in changed:
            print(f"  {namespace}::{field_name}  0x{old_value:X} -> 0x{new_value:X}")
    else:
        print("No offsets needed updating.")

    if unresolved:
        print(f"Could not resolve {len(unresolved)} mapped field(s):")
        for namespace, field_name, class_name in unresolved:
            print(f"  {namespace}::{field_name} (class {class_name})")


if __name__ == "__main__":
    sys.exit(main())