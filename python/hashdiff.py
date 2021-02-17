import os
from hashlib import sha256 as digest
from tqdm import tqdm


os.chdir('ljyys')

correct_sha = set()
my_sha = set()

with open('hash.txt') as r:
    correct_sha.update(r.read().splitlines())

os.chdir('pics')
for fn in tqdm(os.listdir('.')):
    with open(fn, 'rb') as r:
        my_sha.add(digest(r.read()).hexdigest())

print(correct_sha - my_sha)
