import sys, os.path, shutil

outdir = os.path.abspath(sys.argv[1])
variant = sys.argv[2]
toolset = "vc" + sys.argv[3][1:]
print ("Install packages into %s with %s variant" % (outdir, variant))

qtdir = "C:/Qt"
qtmudules = ["Core", "Gui", "Widgets", "Svg"]
for m in qtmudules:
    name = "Qt5" + m + ("d" if variant.lower()=="debug" else "")
    for ext in ("dll", "pdb"):
        extname = name + "." + ext
        fullname = os.path.join(qtdir, "bin", extname)
        if not os.path.exists(fullname):
            sys.exit("Module %s not found" % fullname)
        if not os.path.exists(os.path.join(outdir, extname)):
            print("Copying %s" % extname)
            shutil.copy(fullname, outdir)

#boost_root = "C:/Boost"
#boost_ver = "1.59"
#boost_modules = ["unit_test_framework", "regex", "thread", "system", "date_time", "chrono"]
#for m in boost_modules:
#    name = "boost_" + m + "-" + toolset + "-mt-" + ("gd-" if variant.lower()=="debug" else "") + boost_ver.replace('.', '_')
#    for ext in (["dll"]):
#        extname = name + "." + ext
#        fullname = os.path.join(boost_root, "lib", extname)
#        if not os.path.exists(fullname):
#            sys.exit("Module %s not found" % fullname)
#        if not os.path.exists(os.path.join(outdir, extname)):
#            print("Copying %s" % extname)
#            shutil.copy(fullname, outdir)
        