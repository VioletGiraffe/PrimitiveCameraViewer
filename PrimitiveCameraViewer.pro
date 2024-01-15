TEMPLATE = subdirs

SUBDIRS = app cpputils qtutils github-releases-autoupdater cpp-template-utils

cpputils.subdir = cpputils

qtutils.subdir = qtutils
qtutils.depends = cpputils

github-releases-autoupdater.subdir = github-releases-autoupdater
github-releases-autoupdater.depends = cpputils

app.subdir  = app
app.depends = qtutils github-releases-autoupdater
