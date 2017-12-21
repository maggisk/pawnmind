from random import randint

def random_pick(l):
    # this concept is used for picking chess move by random when
    # there are more than 1 equally good best moves
    v = None
    for i, item in enumerate(l):
        if randint(0, i) == 0:
            v = item
    return v

SIZE = 10
N_PICKS = 100000

l = list(range(SIZE))
n = [0] * len(l)

for i in range(N_PICKS):
    n[random_pick(l)] += 1

# if random_pick is fair, then each value in n should be close to N_PICKS / SIZE
print n
