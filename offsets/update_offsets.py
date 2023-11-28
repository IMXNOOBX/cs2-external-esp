import sys
import requests
import json
import re

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
        for commit in commit_data:
            commit_message = commit['commit']['message']
            build_match = re.search(r'\bGame Update (\d+)(?: \(\d+\))?\b', commit_message)
            if build_match:
                build_number = int(build_match.group(1))
                break

with open(dest_path, 'r') as dest_file:
    dest_data = json.load(dest_file)
    
if dest_data["build_number"] == int(build_number):
    print("There are no updates in the remote reposioty")
    sys.exit(1)  # Exit so we know that there hasnt been any updates and the current offsets are up to date

dest_data["build_number"] = int(build_number)

dest_data["dwBuildNumber"] = source_data["engine2_dll"]["data"]["dwBuildNumber"]["value"]
dest_data["dwLocalPlayerController"] = source_data["client_dll"]["data"]["dwLocalPlayerController"]["value"]
dest_data["dwEntityList"] = source_data["client_dll"]["data"]["dwEntityList"]["value"]
dest_data["dwViewMatrix"] = source_data["client_dll"]["data"]["dwViewMatrix"]["value"]
dest_data["dwPlantedC4"] = source_data["client_dll"]["data"]["dwPlantedC4"]["value"]

with open(dest_path, 'w') as dest_file:
    json.dump(dest_data, dest_file, indent=4)

print("Offsets updated in the local file.") # We dont exit so we know that the game has been updated and that we have to commit the new offsets
