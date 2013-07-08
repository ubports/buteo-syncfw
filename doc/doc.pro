DOXYGEN_BIN=doxygen

QMAKE_EXTRA_TARGETS += doc

QT -= gui

doc.target = doc

isEmpty(DOXYGEN_BIN) {
    doc.commands = @echo "Unable to detect doxygen in PATH"
} else {
    doc.commands = @$${DOXYGEN_BIN} Doxyfile;
}

#doc.depends = FORCE

QMAKE_CLEAN += ./doc/html/*

# Install rules
htmldocs.files = ./doc/html/*
htmldocs.path = /usr/share/doc/sync-fw-doc/
htmldocs.CONFIG += no_check_exist

INSTALLS += htmldocs
