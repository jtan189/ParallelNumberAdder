import sys

sum = 0
fnames = []

if len(sys.argv) > 1:
    fnames.append(sys.argv[1])
else:
    fnames.append('data/file1.dat')
    fnames.append('data/file2.dat')
    fnames.append('data/file3.dat')
    fnames.append('data/file4.dat')

print("----------------------")
for fname in fnames:
    f = open(fname)
    for line in f:
        sum += int(line)
    print("File: %s" % (fname))
    print("Total sum: %i" % (sum))
    print("----------------------")
