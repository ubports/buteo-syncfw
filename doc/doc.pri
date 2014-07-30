DOXYGEN_BIN = $$system(command -v doxygen)
isEmpty(DOXYGEN_BIN):error("Unable to detect doxygen in PATH")

doc.CONFIG = phony
doc.commands = cd $${PWD} && $${DOXYGEN_BIN} Doxyfile
QMAKE_EXTRA_TARGETS += doc
QMAKE_CLEAN += $${PWD}/html/* $${PWD}/buteo-syncfw.tag

# Install rules
htmldocs.files = $${PWD}/html/
htmldocs.path = /usr/share/doc/buteo-syncfw-doc/
INSTALLS += htmldocs
