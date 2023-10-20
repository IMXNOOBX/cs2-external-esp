import sys
import requests
import json

source_url = "https://github.com/a2x/cs2-dumper/raw/main/generated/offsets.json"
commits_url = "https://api.github.com/repos/a2x/cs2-dumper/commits"
dest_path = "./offsets/offsets.json"

# Fetch the source JSON
source_response = requests.get(source_url)
source_data = source_response.json()

# Fetch build Number
response = requests.get(commits_url)
build_number = 0
if response.status_code == 200:
    commit_data = response.json()
    if commit_data:
        last_commit_message = commit_data[0]['commit']['message']
        build_number = last_commit_message.split()[-1]

with open(dest_path, 'r') as dest_file:
    dest_data = json.load(dest_file)
    
if dest_data["build_number"] == int(build_number):
    print("There are no updates in the remote reposioty")
    sys.exit(1)  # Exit so we know that there hasnt been any updates and the current offsets are up to date

dest_data["build_number"] = int(build_number)

dest_data["dwBuildNumber"] = source_data["Engine2Dll"]["dwBuildNumber"]["value"]
dest_data["dwLocalPlayer"] = source_data["ClientDll"]["dwLocalPlayerPawn"]["value"]
dest_data["dwEntityList"] = source_data["ClientDll"]["dwEntityList"]["value"]
dest_data["dwViewMatrix"] = source_data["ClientDll"]["dwViewMatrix"]["value"]

with open(dest_path, 'w') as dest_file:
    json.dump(dest_data, dest_file, indent=4)

print("Offsets updated in the local file.") # We dont exit so we know that the game has been updated and that we have to commit the new offsets
