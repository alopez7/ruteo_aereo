costs = {}
macros = {94: 40, 95: 41, 96: 42, 97: 43, 98: 44, 99: 45, 100: 46, 101: 47}
with open("orders.txt", "r") as f:
    f.readline()
    for line in f.readlines():
        l = line.strip().split(" ")
        n_id = int(l[0])
        m_id = int(l[1])
        macros[n_id] = m_id

with open("costs.txt", "r") as f:
    for line in f.readlines():
        l = line.strip().split(" ")
        n0 = int(l[0]) - 1
        n1 = int(l[1]) - 1
        if n0 == -1:
            n0 = 93 + int(l[2])
        if n1 == 94:
            n1 = 97 + int(l[2])
        m0 = macros[n0]
        m1 = macros[n1]
        if not m0 in costs:
            costs[m0] = {}
        pair = [float(l[3]), float(l[4])]
        if m1 in costs[m0]:
            if pair[0] != costs[m0][m1][0] or pair[1] != costs[m0][m1][1]:
                print("Error: {}, {}".format(pair, costs[m0][m1]))
                print("m0: {}, m1: {}, a: {}".format(m0, m1, int(l[2])))
                input()
        costs[m0][m1] = pair

for m0 in costs:
    for m1 in costs[m0]:
        print(m0, m1, costs[m0][m1][0], costs[m0][m1][1])
