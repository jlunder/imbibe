import math


def color_i(c):
    r, g, b = c
    return 0.21 * r + 0.72 * g + 0.07 * b


def color_dist(c1, c2):
    r1, g1, b1 = c1
    r2, g2, b2 = c2
    return math.sqrt(
        (r2 - r1) ** 2
        + (g2 - g1) ** 2
        + (b2 - b1) ** 2
        + (3 * (color_i(c1) - color_i(c2))) ** 2
    )


colors = [
    (0, 0, 0),
    (2, 0, 0),
    (0, 2, 0),
    (2, 2, 0),
    (0, 0, 2),
    (2, 0, 2),
    (0, 2, 2),
    (2, 2, 2),
    (1, 1, 1),
    (3, 1, 1),
    (1, 3, 1),
    (3, 3, 1),
    (1, 1, 3),
    (3, 1, 3),
    (1, 3, 3),
    (3, 3, 3),
]

# for ci in colors:
#     print(", ".join(["%5.2f" % color_dist(ci, cj) for cj in colors]))

# for i in range(len(colors)):
#     ci = colors[i]
#     black_dist = color_dist(ci, colors[0])
#     dists = [(-color_dist(ci, cj), j, cj,) for j, cj in enumerate(colors) if color_dist(ci, cj) < black_dist]
#     dists.sort()
#     print(i, ': ', [j for dij, j, cj in dists], sep='')


def pad_oct(j):
    s = oct(j).replace("o", "")
    return "0" * (3 - len(s)) + s


for k in range(0, 16):
    dists = []
    for i in range(len(colors)):
        ci = colors[i]
        ri, gi, bi = ci
        ck = ((ri * k) / 15, (gi * k) / 15, (bi * k) / 15)

        best = 0
        cbest = colors[0]
        dibest = color_dist(ck, cbest)
        for j in range(1, len(colors)):
            cj = colors[j]
            dij = color_dist(ck, cj)
            if dij < dibest:
                best = j
                cbest = cj
                dibest = dij
        dists.append((dibest, best, cbest))
    # print(i, ': ', [j for dij, j, cj in dists], sep='')
    print("  {" + ", ".join([pad_oct(j) for dij, j, cj in dists]) + "},")
