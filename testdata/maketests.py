from itertools import product

_compl = dict(zip('ACGT', 'TGCA'))
_ch = dict(zip('ACGT', 'CTAG'))

def c(s):
    return ''.join(_compl[si] for si in s)

def rc(s):
    return c(s[::-1])

def ch(s):
    return ''.join(_ch[si] for si in s)


refseqs = [s.rstrip() for s in open('ref.fa').readlines()[1::2]]
out = open('refq.fa', 'w')

for s in refseqs:
    for i, (rc_, oend, ostart) in enumerate(product((0, 1), (None, -4), (None, 5)), 1):
        label = str(i)
        q = s
        if ostart:
            q = ch(q[:ostart]) + q[ostart:]
        if oend:
            q = q[:oend] + ch(q[oend:])
        if rc_:
            q = rc(q)
            label = 'r' + label
        out.write('>%s\n%s\n' %(label, q))

out.close()
