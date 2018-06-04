from itertools import product

_c = dict(zip('ACGT', 'TGCA'))

def c(s):
    return ''.join(_c[si] for si in s)

def rc(s):
    return c(s[::-1])


refseqs = [s.rstrip() for s in open('ref.fa').readlines()[1::2]]
out = open('refq.fa', 'w')

for s in refseqs:
    for i, (rc_, ostart, oend) in enumerate(product((0, 1), (0, 5), (0, -4)), 1):
        label = str(i)
        q = s
        q = rc(q[:ostart]) + q[ostart:]
        q = q[:oend] + rc(q[oend:])
        if rc_:
            q = rc(q)
            label = 'r' + label
        out.write('>%s\n%s\n' %(label, q))

out.close()
