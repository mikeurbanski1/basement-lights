rm log.txt && nohup ./ngrok http --log log.txt 7626 &
ask smapi get-skill-manifest -s 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0' -g development | jq ".manifest.apis.custom.endpoint.uri = \"$url\"" > config.json
ask smapi update-skill-manifest -s 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0' -g development --manifest "$(cat config.json)"
