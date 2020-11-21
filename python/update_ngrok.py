import subprocess
import os
import time

if os.path.exists('log.txt'):
    os.remove('log.txt')

ngrok = subprocess.Popen(['ngrok', 'http', '--log', 'log.txt', '7626'], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

print(f'ngrok PID: {ngrok.pid}')

time.sleep(2)

with open('log.txt', 'r') as fp:
    for line in fp.readlines():
        if 'https' in line:
            index = line.index('https')
            url = line[index:]
            print(f'ngrok URL: {url}')
            break


