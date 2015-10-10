import sys, os.path

repodir = os.path.abspath(sys.argv[1])
print(repodir)
sys.path.append(repodir)

import pkgsys.packages

#if sys.argv[2] == "env":
#    pkg.packages.setup()
