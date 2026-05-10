# Extra script: appends gcov to the linker LIBS list for the tests env.
# SCons places LIBS after object files, fixing the symbol-ordering issue
# that causes __gcov_init to be unresolved when -lgcov is in build_flags.
Import("env")
env.Append(LIBS=["gcov"])
