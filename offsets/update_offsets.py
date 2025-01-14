import sys
import requests
import json
import re
import os

urls = {
    "commits": "https://api.github.com/repos/a2x/cs2-dumper/commits",
    "offsets": "https://raw.githubusercontent.com/a2x/cs2-dumper/main/output/offsets.json",
    "client_dll": "https://github.com/a2x/cs2-dumper/raw/refs/heads/main/output/client_dll.json",
}

def get_build_number():
    response = requests.get(urls["commits"])
    if response.status_code == 200:
        commit_data = response.json()
        if commit_data:
            for commit in commit_data:
                commit_message = commit['commit']['message']
                build_match = re.search(r'\bGame [Uu]pdate \((\d+)(?: \(\d+\))?\b', commit_message)
                if build_match:
                    return int(build_match.group(1))
    return 0

def get_raw_file(url):
    response = requests.get(url)
    if response.status_code == 200:
        return response.json()
    return None

def get_path():
    if os.path.isfile("./offsets/offsets.json"):
        return "./offsets/offsets.json"
    elif os.path.isfile("./offsets.json"):
        return "./offsets.json"
    else:
        return None

dest_path = get_path()

if not dest_path:
    print("Invalid path for 'offsets.json'")
    exit(1)

build_number = get_build_number()

if build_number == 0:
    print("Could not find the latest build number.")
    exit(1)

offsets = get_raw_file(urls["offsets"])

if not offsets:
    print("Could not find the latest offsets.")
    exit(1)

with open(dest_path, 'r') as dest_file:
    offsets_json = json.load(dest_file)

print(f"Current build number: {offsets_json['build_number']} vs Latest build number: {build_number}")

if offsets_json["build_number"] == int(build_number):
    print(f"There are no updates in the remote repository after {build_number}.")
    print("Comparing the offsets in the local offsets (dwLocalPlayer/dwViewMatrix) with the remote repository.")

    valid = offsets_json["dwLocalPlayer"] == offsets["client.dll"]["dwLocalPlayerPawn"] and \
                offsets_json["dwViewMatrix"] == offsets["client.dll"]["dwViewMatrix"]

    # Exit so we know that there hasnt been any updates and the current offsets are up to date
    if valid:
        print("Local offsets are up to date.")
        exit(1)

    print("Local offsets (dwLocalPlayer/dwViewMatrix) are outdated, pulling the latest offsets.")

offsets_json["build_number"] = int(build_number)

offsets_json["dwBuildNumber"] = offsets["engine2.dll"]["dwBuildNumber"]
offsets_json["dwLocalPlayer"] = offsets["client.dll"]["dwLocalPlayerPawn"]
offsets_json["dwLocalPlayerController"] = offsets["client.dll"]["dwLocalPlayerController"]
offsets_json["dwEntityList"] = offsets["client.dll"]["dwEntityList"]
offsets_json["dwViewMatrix"] = offsets["client.dll"]["dwViewMatrix"]
offsets_json["dwPlantedC4"] = offsets["client.dll"]["dwPlantedC4"]

client = get_raw_file(urls["client_dll"])

if not client:
    print("Could not find the latest client.dll.")
    exit(1)

client_json_base = client["client.dll"]["classes"]

offsets_json["m_bIsDefusing"] = client_json_base["C_CSPlayerPawn"]["fields"]["m_bIsDefusing"]
offsets_json["m_ArmorValue"] = client_json_base["C_CSPlayerPawn"]["fields"]["m_ArmorValue"]

offsets_json["m_pClippingWeapon"] = client_json_base["C_CSPlayerPawnBase"]["fields"]["m_pClippingWeapon"]
offsets_json["m_flFlashOverlayAlpha"] = client_json_base["C_CSPlayerPawnBase"]["fields"]["m_flFlashOverlayAlpha"]

offsets_json["m_flC4Blow"] = client_json_base["C_PlantedC4"]["fields"]["m_flC4Blow"]
offsets_json["m_flNextBeep"] = client_json_base["C_PlantedC4"]["fields"]["m_flNextBeep"]
offsets_json["m_flTimerLength"] = client_json_base["C_PlantedC4"]["fields"]["m_flTimerLength"]

offsets_json["m_hPlayerPawn"] = client_json_base["CCSPlayerController"]["fields"]["m_hPlayerPawn"]
offsets_json["m_iAccount"] = client_json_base["CCSPlayerController_InGameMoneyServices"]["fields"]["m_iAccount"]
offsets_json["m_pInGameMoneyServices"] = client_json_base["CCSPlayerController"]["fields"]["m_pInGameMoneyServices"]
offsets_json["m_sSanitizedPlayerName"] = client_json_base["CCSPlayerController"]["fields"]["m_sSanitizedPlayerName"]

offsets_json["m_iHealth"] = client_json_base["C_BaseEntity"]["fields"]["m_iHealth"]
offsets_json["m_iTeamNum"] = client_json_base["C_BaseEntity"]["fields"]["m_iTeamNum"]
offsets_json["m_pGameSceneNode"] = client_json_base["C_BaseEntity"]["fields"]["m_pGameSceneNode"]

offsets_json["m_szName"] = client_json_base["CCSWeaponBaseVData"]["fields"]["m_szName"]
offsets_json["m_vOldOrigin"] = client_json_base["C_BasePlayerPawn"]["fields"]["m_vOldOrigin"]
offsets_json["m_vecAbsOrigin"] = client_json_base["CGameSceneNode"]["fields"]["m_vecAbsOrigin"]

with open(dest_path, 'w') as dest_file:
    json.dump(offsets_json, dest_file, indent=4)

print("Offsets updated in the local file.") # We dont exit so we know that the game has been updated and that we have to commit the new offsets
