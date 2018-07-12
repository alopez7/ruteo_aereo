a='''14 70 30 65 24 59 1 36 3 38 23 58 27 62 77
22 71 16 51 10 45 26 61 28 31 63 19 66 54 13 48 20 4 39 55 2 37 78
6 72 11 46 21 56 79
10 73 17 52 9 44 25 60 29 64 80
8 74 18 53 8 33 43 68 81
6 75 34 69 5 40 82
10 76 14 49 35 70 32 67 22 57 83'''

a = a.strip().split('\n')
for i in range(len(a)):
    a[i] = a[i].split(" ")
    for j in range(len(a[i])):
        a[i][j] = int(a[i][j])
        if j >= 2 and j < len(a[i]) - 1:
            a[i][j] -= 1

for i in a:
    for j in i:
        print(j, end = " ")
    print("")
