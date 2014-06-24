.TH FDF 1 "24 Jun 2014" "VERSION"

.SH NAME
fdf \- find duplicate files
.SH SYNOPSIS
\fBfdf\fP [\fIOPTIONS\fP] [\fIFILES\fP]
.SH DESCRIPTION
\fBfdf\fP allows finds duplicate files in given file trees.
.SH OPTIONS
.TP
\fB-d\fP \fINUM\fP, \fB--tree-depth\fP \fINUM\fP
Search filetrees up to NUM depth.
.TP
\fB--help\fP, \fB--usage\fP
Show brief help and exit.
.TP
\fB-s\fP, \fB--include-symlinks\fP
Handle symlinks as if they were the file they are linking to. The default is to
ignore symlinks.
.TP
\fB-V\fP, \fB--version\fP
Show program version and exit.
.SH COPYRIGHT
Copyright (c) 2014 Tuomo Hartikainen. This is free software; see the source
files for copying conditions.
.SH AUTHOR
Tuomo Hartikainen <tth@harski.org>.
