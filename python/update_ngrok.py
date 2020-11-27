import subprocess
import os
import time
import json
import sys
import re

pattern = re.compile(r'https://[a-z0-9]+\.ngrok\.io')

while True:
    if os.path.exists('log.txt'):
        print('Removing old log.txt')
        os.remove('log.txt')
    else:
        print('log.txt did not exist')

    print('Starting ngrok')
    ngrok = subprocess.Popen(['ngrok', 'http', '--log', 'log.txt', '7626'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    print(f'ngrok PID: {ngrok.pid}')

    time.sleep(5)  # slight delay for ngrok to run

    url = None

    iterations = 1
    while not url:  # it may start up before internet is available, so loop until we find the new entry
        with open('log.txt', 'r') as fp:
            for line in fp.readlines():
                matches = pattern.findall(line)
                if matches:
                    url = matches[0]
                    break
        time.sleep(1)
        iterations += 1

    url = url.rstrip()
    print(f'Found url" {url} (iterations: {iterations}')

    with open('url.txt', 'w') as fp:
        fp.write(url)

    get_manifest = subprocess.Popen(['ask', 'smapi', 'get-skill-manifest', '-s', 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0', '-g', 'development'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    stdout, stderr = get_manifest.communicate()

    if stderr:
        print('Got stderr output while getting manifest', file=sys.stderr)
        print(stderr, file=sys.stderr)
        ngrok.kill()
        exit(1)

    manifest_config = json.loads(stdout)

    print(manifest_config)

    manifest_config['manifest']['apis']['custom']['endpoint']['uri'] = url

    print(manifest_config)

    update_manifest = subprocess.Popen(['ask', 'smapi', 'update-skill-manifest', '-s', 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0', '-g', 'development', '--manifest', json.dumps(manifest_config)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = update_manifest.communicate()

    if stderr:
        print('Got stderr output while updating manifest', file=sys.stderr)
        print(stderr, file=sys.stderr)

    print(stdout)

    # send init notification
    curl = subprocess.Popen(['curl', f'{url}/hello'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = curl.communicate()
    if stderr:
        print('Got stderr output while sending curl command', file=sys.stderr)
        print(stderr, file=sys.stderr)

    print(stdout)

    print('Sleeping for 7 hours (25200 seconds)')
    time.sleep(25200)
    print('Killing ngrok')
    ngrok.kill()
    time.sleep(5)