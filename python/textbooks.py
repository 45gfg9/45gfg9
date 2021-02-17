import os
import time
from threading import Thread
from urllib import parse, request

from retry import retry
from pyquery import PyQuery as pq

import ssl

ssl._create_default_https_context = ssl._create_unverified_context
del ssl

MAX_THREADS = 16
TEACHERS_BOOK = False

books_pending = []
threads_running = []
books_failed = []

FOLDER_NAME = '普通高中课程标准实验教科书'
os.makedirs(FOLDER_NAME, exist_ok=True)
os.chdir(FOLDER_NAME)

end = False


def downloadManager():
    while True:
        while books_pending and len(threads_running) < MAX_THREADS:
            furl, subj, fname = books_pending.pop(0)
            thread = Thread(target=download, args=(furl, subj, fname))
            thread.start()
            threads_running.append(thread)
        if threads_running:
            for thread in threads_running:
                if not thread.is_alive():
                    threads_running.remove(thread)
        if end and not books_pending and not threads_running:
            break
        time.sleep(0.1)


def download(url, subj, bname):
    ERRORS = (ConnectionError, OSError)

    @retry(ERRORS, tries=3)
    def idw():
        print('Downloading', bname)
        request.urlretrieve(url, filename=f'{subj}/{bname}.pdf')
        print('Downloaded', bname)

    try:
        idw()
    except ERRORS:
        print(f'Download {bname} failed.')
        books_failed.append(bname)


def main():
    global end
    url = 'https://bp.pep.com.cn/jc/'
    print('Starting download manager thread')
    dmgr = Thread(target=downloadManager)
    dmgr.start()

    try:
        doc = pq(str(request.urlopen(url).read(), encoding='utf-8'))
        rootDiv = doc('div.list_sjzl_jcdzs2020')[6]
        uls = rootDiv.findall('ul')
        lis = map(lambda e: e.findall('li')[1 if TEACHERS_BOOK else 0], uls)
        tas = map(lambda e: e.find('a'), lis)
        uris = map(lambda e: (e.text, e.get('href')), tas)

        for typ, uri in uris:
            os.makedirs(typ, exist_ok=True)
            surl = parse.urljoin(url, uri)
            sub = pq(str(request.urlopen(surl).read(), encoding='utf-8'))
            uls = sub('ul')
            for ul in uls:
                lis = ul.findall('li')
                ats = map(lambda e: e.find('a').get('title'), lis)
                divs = map(lambda e: e.find('div'), lis)
                buris = map(lambda e: e.findall('a')[1].get('href'), divs)
                for at, buri in zip(ats, buris):
                    dest = parse.urljoin(surl, buri)
                    books_pending.append((dest, typ, at))
    finally:
        end = True
        dmgr.join()


if __name__ == '__main__':
    main()
