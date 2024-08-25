import os, random

all_names = [
    s.encode() for s in sorted(set(os.listdir("data") + os.listdir("testdata")))
]


def djb2(s):
    hash = 5381
    for x in s:
        hash = ((hash << 5) + hash + x) % 65536
    return hash


def sdbm(s):
    hash = 0
    for x in s:
        hash = ((hash << 6) + (hash << 16) - hash + x) % 65536
    return hash


def fletcher16(s):
    sum1 = 0
    sum2 = 0
    for x in s:
        sum1 += x
        while sum1 >= 255:
            sum1 -= 255
        sum2 += sum1
        while sum2 >= 255:
            sum2 -= 255
    return (sum2 << 8) | sum1


def fletcher32(s):
    sum1 = 0
    sum2 = 0
    phase = 0
    if len(s) % 2 == 1:
        s += bytes(b"\x00")
    for x in s:
        if phase == 0:
            sum1 += x
        if phase == 1:
            sum1 += x << 8
            while sum1 >= 65535:
                sum1 -= 65535
            sum2 += sum1
            if sum2 >= 65535:
                sum2 -= 65535
    return (sum2 << 16) | sum1


def check_hash(hname, f, names, quiet=False):
    if not quiet:
        print("%s:" % (hname,))
    hashed = {}
    for s in names:
        h = f(s)
        # print(h, '=', s)
        if h not in hashed:
            hashed[h] = []
        hashed[h].append(s)
    n = 0
    max_n = 0
    for h, ss in hashed.items():
        max_n = max(len(ss), max_n)
        if len(ss) > 1:
            # print("  collision: %04X = %r" % (h, ss,))
            n += len(ss) - 1
    if not quiet:
        if n == 0:
            print("  no collisions")
        else:
            print("  %d collisions, max overlap %d" % (n, max_n))
        print()
    return n


def make_random_names(n_strs):
    chars = b"._# " + bytes([ord("a") + x for x in range(26)])
    random_names = set()
    while len(random_names) < n_strs:
        random_names.add(
            bytes([random.choice(chars) for _ in range(random.randint(8, 20))])
        )
    return list(random_names)


def check_random(hname, f):
    n_strs = 100
    n_fails = 0
    n_salt_fails = 0
    n_samples = 2000
    n_salt_tries = 1000
    for i in range(n_samples):
        random_names = make_random_names(n_strs)
        if check_hash(hname, f, random_names, quiet=True) > 0:
            n_fails += 1
        salt = 0
        salt_b = b"\x00\x00"
        salted_names = [salt_b + b for b in random_names]
        for _ in range(n_salt_tries):
            if check_hash(hname, f, salted_names, quiet=True) == 0:
                break
            salt = (salt + 1) & 0xFFFF
            salt_b = bytes([salt & 0xFF, salt >> 8])
            salted_names = [salt_b + b for b in random_names]
        else:
            n_salt_fails += 1
            # print("Salt fail on this wordlist:")
            # for b in sorted(random_names):
            #     print("  " + repr(b))
    print(
        "%s: random names, %d strings: P(fail) = %g; P(salt_fail) = %g"
        % (
            hname,
            n_strs,
            n_fails / n_samples,
            n_salt_fails / n_samples,
        )
    )


def max_bin_depth(n_bins, hashes):
    bins = [0] * n_bins
    for h in hashes:
        bins[h % n_bins] += 1
    return max(bins)


def max_bin_depth_flat(n_bins, hashes):
    bins = [0] * n_bins
    max_search = 0
    for h in hashes:
        for i in range(n_bins):
            if bins[(h + i) % n_bins] == 0:
                bins[(h + i) % n_bins] = 1
                break
        max_search = max(i, max_search)
    return max_search


def check_binning(hname, f):
    n_strs = 100
    n_reps = 1000

    results = {}
    for _ in range(n_reps):
        hashes = list(map(f, make_random_names(n_strs)))
        for n_bins in [32, 37, 64, 67, 128, 131, 256, 257, 512, 521]:
            if not n_bins in results:
                results[n_bins] = 0
            results[n_bins] += max_bin_depth(n_bins, hashes)
    print(
        "%s: random names, %d strings: %s"
        % (hname, n_strs, {k: v / n_reps for k, v in results.items()})
    )

    results = {}
    for _ in range(n_reps):
        hashes = list(map(f, make_random_names(n_strs)))
        for n_bins in [32, 37, 64, 67, 128, 131, 256, 257, 512, 521]:
            if n_bins < n_strs:
                continue
            if not n_bins in results:
                results[n_bins] = 0
            results[n_bins] += max_bin_depth_flat(n_bins, hashes)
    print(
        "%s: random names, %d strings, flat: %s"
        % (hname, n_strs, {k: v / n_reps for k, v in results.items()})
    )


# check_hash("djb2", djb2, all_names)
# check_hash("sdbm", sdbm, all_names)
# check_hash("fletcher16", fletcher16, all_names)
# check_hash("fletcher32", fletcher16, all_names)

# check_random("djb2", djb2)
# check_random("sdbm", sdbm)
# check_random("fletcher16", fletcher16)
# check_random("fletcher32", fletcher16)

check_binning("djb2", djb2)
check_binning("sdbm", sdbm)
check_binning("fletcher16", fletcher16)
check_binning("fletcher32", fletcher16)
