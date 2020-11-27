import logging
import subprocess
import os
import time
import json
import sys
import re

pattern = re.compile(r'https://[a-z0-9]+\.ngrok\.io')

logger = logging.getLogger(__name__)
logger.setLevel('DEBUG')
logger.addHandler(logging.StreamHandler())

while True:
    if os.path.exists('log.txt'):
        logger.debug('Removing old log.txt')
        os.remove('log.txt')
    else:
        logger.debug('log.txt did not exist')

    logger.info('Starting ngrok')
    ngrok = subprocess.Popen(['ngrok', 'http', '--log', 'log.txt', '7626'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    logger.info(f'ngrok PID: {ngrok.pid}')

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
    logger.info(f'Found url: {url} (iterations: {iterations})')

    with open('url.txt', 'w') as fp:
        fp.write(url)

    get_manifest = subprocess.Popen(['ask', 'smapi', 'get-skill-manifest', '-s', 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0', '-g', 'development'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    stdout, stderr = get_manifest.communicate()

    if stderr:
        logger.critical('Got stderr output while getting manifest', file=sys.stderr)
        logger.critical(stderr, file=sys.stderr)
        ngrok.kill()
        exit(1)

    manifest_config = json.loads(stdout)

    logger.info('Current manifest config:')
    logger.info(json.dumps(manifest_config, indent=2))

    manifest_config['manifest']['apis']['custom']['endpoint']['uri'] = url

    logger.info('New manifest config:')
    logger.info(json.dumps(manifest_config, indent=2))

    update_manifest = subprocess.Popen(['ask', 'smapi', 'update-skill-manifest', '-s', 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0', '-g', 'development', '--manifest', json.dumps(manifest_config)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = update_manifest.communicate()

    if stderr:
        logger.critical('Got stderr output while updating manifest', file=sys.stderr)
        logger.critical(stderr, file=sys.stderr)
        ngrok.kill()
        exit(1)

    logger.info('Update skill response:')
    logger.info(stdout)

    # send init notification
    curl = subprocess.Popen(['curl', f'{url}/hello'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = curl.communicate()

    logger.error(f'Curl stderr: {stderr}')
    logger.info(stdout)

    logger.info('Sleeping for 7 hours (25200 seconds)')
    time.sleep(25200)
    logger.info('Killing ngrok')
    ngrok.kill()
    time.sleep(5)