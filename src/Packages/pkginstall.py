import sys, os.path

repodir = os.getcwd()
pkgsysdir = os.path.join(repodir, "pkgsys")

sys.path.append(pkgsysdir)
import packages

packages.setup("install", ["cpplint"])
