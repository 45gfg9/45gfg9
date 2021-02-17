import os
import time
import sys
from threading import Thread
from urllib import request

from hashlib import sha256 as digest
from filetype import filetype
from retry import retry

URL = 'https://img.ljyys.ml:10131/ljyysnmsl'

threads_running = []
have_hashes = []


MAX_THREADS = int(sys.argv[1])

VARIANT = URL.rsplit('/', 1)[1]
ROOT_DIR = 'ljyys'
HASH_NAME = f'{VARIANT}_my_sha256.txt'
SUB_DIR = f'{VARIANT}_sha256_pics'

os.makedirs(os.path.join(ROOT_DIR, SUB_DIR), exist_ok=True)
os.chdir(ROOT_DIR)


end = False


def timePrint(*args, **kwargs):
    print(time.strftime('[%Y/%m/%d %H:%M:%S]', time.localtime()), *args, **kwargs)


def downloadManager():
    while True:
        while not end and len(threads_running) < MAX_THREADS:
            thread = Thread(target=download)
            thread.start()
            threads_running.append(thread)
        if threads_running:
            for thread in threads_running:
                if not thread.is_alive():
                    threads_running.remove(thread)
                    if end:
                        print('Left', len(threads_running), 'threads alive')
        if end and not threads_running:
            print('Finishing up')
            with open(HASH_NAME, 'w') as fin:
                for h in sorted(have_hashes):
                    print(h, file=fin)
            break
        time.sleep(0.1)


def download():
    ERRORS = (ConnectionError, OSError)

    @retry(ERRORS, tries=2)
    def idw():
        resp = request.urlopen(URL, timeout=10)
        exp_len = int(resp.info()['Content-Length'])
        content = resp.read()
        if exp_len != len(content):
            return
        hashed = digest(content).hexdigest()
        if hashed in have_hashes:
            return
        have_hashes.append(hashed)
        timePrint(f'Received {hashed}, currently {len(have_hashes)}')
        ext = filetype.guess_extension(content)
        filename = f'{hashed}.{ext}'
        dumpBlob(filename, content)

    try:
        idw()
    except ERRORS:
        timePrint('Connection Error occurred', file=sys.stderr)


def dumpBlob(filename, content):
    with open(os.path.join(SUB_DIR, filename), 'wb') as out:
        out.write(content)


def main():
    global end

    if os.path.exists(HASH_NAME):
        with open(HASH_NAME, 'r') as hashes:
            have_hashes.extend(hashes.read().splitlines())

    print('Starting download manager thread')
    dmgr = Thread(target=downloadManager)
    dmgr.start()

    print(f'Begin with {len(have_hashes)}, max {MAX_THREADS} running threads')

    try:
        while True:
            time.sleep(3600)
    except KeyboardInterrupt:
        pass
    finally:
        end = True
        print('Waiting for download manager to end')
        dmgr.join()


if __name__ == '__main__':
    main()
