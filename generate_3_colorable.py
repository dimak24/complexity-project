import random
import sys


# n -- number of vertices
# p -- edge probability
def generate_3_colorable(n, p):
    parts = [[], [], []]
    for i in range(n):
        parts[random.randint(0, 2)].append(i)

    random.shuffle(parts)
    random.shuffle(parts[0])
    random.shuffle(parts[1])
    random.shuffle(parts[2])

    print(n)
    ans = [[] for i in range(n)]

    for i in parts[0]:
        for j in parts[1]:
            if random.random() <= p:
                ans[i].append(str(j))
                ans[j].append(str(i))

    for i in parts[0]:
        for j in parts[2]:
            if random.random() <= p:
                ans[i].append(str(j))
                ans[j].append(str(i))

    for i in parts[2]:
        for j in parts[1]:
            if random.random() <= p:
                ans[i].append(str(j))
                ans[j].append(str(i))

    for i in range(n):
        print('{}: {}'.format(len(ans[i]), ' '.join(ans[i])))


try:
    generate_3_colorable(int(sys.argv[1]), float(sys.argv[2]))
except Exception as e:
    print("error: expected 2 arguments (vertices number (int) and probability(float))")
