import random
import sys


# n -- number of vertices
# p -- edge probability
def generate_non_3_colorable(n, p):
    assert(n >= 4)
    print(n)

    ans = [[] for i in range(n)]

    clique = random.sample(range(0, n), 4)
    while len(set(clique)) != 4:
        clique = random.sample(range(0, n), 4)

    ans[clique[0]].extend(clique[1:])
    ans[clique[1]].extend(clique[:1] + clique[2:])
    ans[clique[2]].extend(clique[:2] + clique[3:])
    ans[clique[3]].extend(clique[:3])

    for i in range(n):
        for j in range(n):
            if i != j and not (i in clique and j in clique) and random.random() <= p:
                ans[i].append(j)
                ans[j].append(i)

    for i in range(n):
        print('{}: {}'.format(len(ans[i]), ' '.join([str(v) for v in ans[i]])))

 
try:
    generate_non_3_colorable(int(sys.argv[1]), float(sys.argv[2]))
except Exception as e:
    print("error: expected 2 arguments (vertices number (int) and probability(float))")
