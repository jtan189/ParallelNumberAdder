import sys, time

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
    start_time = time.time()
    f = open(fname)
    count = 0
    total_sum = 0
    for line in f:
        total_sum += int(line)
        count += 1
    print("Filename: %s" % (fname))
    print("Num lines: %i" % (count))
    print("Total sum: %i" % (total_sum))
    print("Execution time: %f seconds" % (time.time() - start_time))
    print("----------------------")
