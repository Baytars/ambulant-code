# Scan an Apple header file, generating a Python file of generator calls.

import sys
import os
from bgenlocations import TOOLBOXDIR, BGENDIR
sys.path.append(BGENDIR)

# ADD object typenames here

def main():
    input = [
            ]
    output = "ambulantgen.py"
    #defsoutput = TOOLBOXDIR + LONG + ".py"
    #scanner = MyScanner(input, output, defsoutput)
    #scanner.scan()
    #scanner.gentypetest(SHORT+"typetest.py")
    #scanner.close()
    #print "=== Testing definitions output code ==="
    #execfile(defsoutput, {}, {})
    print "=== Done scanning and generating, now importing the generated code... ==="
    exec "import ambulantsupport"
    print "=== Done.  It's up to you to compile it now! ==="


if __name__ == "__main__":
    main()
