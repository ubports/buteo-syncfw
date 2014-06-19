include(tests_common.pri)

pro_file_basename = $$basename(_PRO_FILE_)
pro_file_basename ~= s/\\.pro$//

TEMPLATE = app
TARGET = $${pro_file_basename}

HEADERS = $${pro_file_basename}.h
SOURCES = $${pro_file_basename}.cpp

target.path = $${INSTALL_TESTDIR}/$${tests_subdir}
INSTALLS += target

#check.depends = all
#check.commands = '\
#    cd "$${PWD}" \
#    && export LD_LIBRARY_PATH="$${OUT_PWD}/$${tests_subdir_r}/../src:\$\${LD_LIBRARY_PATH}" \
#    && $${OUT_PWD}/$${TARGET}'
#check.CONFIG = phony
#QMAKE_EXTRA_TARGETS += check
