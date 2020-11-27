import logging.handlers
import subprocess
import os
import time
import json
import re


# welcome to the jankiest script ever

pattern = re.compile(r'https://[a-z0-9]+\.ngrok\.io')  # regex pattern for ngrok url in ngrok log file

logger = logging.getLogger(__name__)
logger.setLevel('DEBUG')

formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')

handler = logging.StreamHandler()
handler.setFormatter(formatter)
logger.addHandler(handler)

handler = logging.handlers.RotatingFileHandler('logs/ngrok.log', maxBytes=1024 * 10, backupCount=5)
handler.setFormatter(formatter)
logger.addHandler(handler)

ngrok_log = 'logs/ngrok_exec.log'

first_loop = True


def send_status_update(url, status):
    curl = subprocess.Popen(['curl', f'{url}/startup/{status}', '-X', 'POST'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = curl.communicate()

    if stderr:
        logger.info(f'Curl stderr: {stderr}')  # normally just the curl status output, but log it in case

    logger.info(stdout)


try:
    while True:
        # We are going to search this log for the url, as this is the best (only?) way to get output from the
        # ngrok process. Remove the old one so we don't find the old url.
        if os.path.exists(os.path.join(ngrok_log)):
            logger.debug('Removing old ngrok log')
            os.remove(ngrok_log)
        else:
            logger.debug('ngrok log did not exist')

        logger.info('Starting ngrok')
        ngrok = subprocess.Popen(['ngrok', 'http', '--log', ngrok_log, '7626'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        logger.info(f'ngrok PID: {ngrok.pid}')

        time.sleep(5)  # slight delay for ngrok to run

        url = None

        iterations = 1
        while not url:  # this script may start up before internet is available, so loop until we find the new entry
            with open(ngrok_log, 'r') as fp:
                for line in fp.readlines():
                    matches = pattern.findall(line)
                    if matches:
                        url = matches[0]
                        break
            time.sleep(1)
            iterations += 1

        url = url.rstrip()
        logger.info(f'Found url: {url} (iterations: {iterations})')

        if first_loop:
            send_status_update(url, 'ngrok_running')

        # this just makes the URL easy to find in case we need it.
        with open('logs/url.txt', 'w') as fp:
            fp.write(url)
            logger.debug('Wrote new URL to logs/url.txt')

        # get the current manifest, which we will then update with the new URL
        get_manifest = subprocess.Popen(['ask', 'smapi', 'get-skill-manifest', '-s', 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0', '-g', 'development'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        stdout, stderr = get_manifest.communicate()

        # We do not expect any stderr output, so this would indicate a failure
        if stderr:
            logger.critical('Got stderr output while getting manifest')
            logger.critical(stderr)
            ngrok.kill()
            exit(1)

        manifest_config = json.loads(stdout)

        if first_loop:
            send_status_update(url, 'manifest_retrieved')

        logger.info('Current manifest config:')
        logger.info(json.dumps(manifest_config, indent=2))

        manifest_config['manifest']['apis']['custom']['endpoint']['uri'] = url

        logger.info('New manifest config:')
        logger.info(json.dumps(manifest_config, indent=2))

        update_manifest = subprocess.Popen(['ask', 'smapi', 'update-skill-manifest', '-s', 'amzn1.ask.skill.15dc0b67-8d15-4262-a35d-8333ae4568f0', '-g', 'development', '--manifest', json.dumps(manifest_config)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = update_manifest.communicate()

        # On a normal execution, we get this stderr warning; it's not a true error
        if stderr and not stderr.startswith(b'[Warn]: This is an asynchronous operation'):
            logger.critical('Got unexpected stderr output while updating manifest')
            logger.critical(stderr)
            ngrok.kill()
            exit(1)
        elif stderr:
            logger.warning(stderr)

        logger.info('Update skill response:')
        logger.info(stdout)

        if first_loop:
            send_status_update(url, 'manifest_updated')

        time.sleep(5)  # give some time for the update to actually take effect so that the init command is accurate

        # send init notification
        send_status_update(url, 'initialized')

        first_loop = False

        logger.info('Sleeping for 7 hours (25200 seconds)')
        time.sleep(25200)
        logger.info('Killing ngrok')
        ngrok.kill()
        time.sleep(5)

except Exception as e:
    logger.critical('An unrecoverable error occurred', exc_info=e)
