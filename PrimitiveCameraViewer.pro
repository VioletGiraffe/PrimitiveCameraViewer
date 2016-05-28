TEMPLATE = subdirs

SUBDIRS = app cpputils qtutils

cpputils.subdir = cpputils

qtutils.subdir = qtutils
qtutils.depends = cpputils

app.subdir  = app
app.depends = qtutils
