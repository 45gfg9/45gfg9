import os
from hashlib import md5 as digest
from tqdm import tqdm


digests = []


def main():
    os.chdir('ljyys')

    os.chdir('pics')
    for fn in tqdm(os.listdir('.')):
        with open(fn, 'rb') as f:
            k = digest(f.read()).hexdigest()
        if k in digests:
            os.remove(fn)
        else:
            digests.append(k)
            ext = fn.split('.')[1]
            os.rename(fn, f'{k}.{ext}')

    with open('got_hashes.txt', 'w') as f:
        for h in tqdm(digests):
            print(h, file=f)


if __name__ == '__main__':
    main()
