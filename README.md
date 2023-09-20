# touch

touch(1) for Windows

## NAME

**touch** — change file access and modification times

## SYNOPSIS

**touch** [**-acm**][ **-r** _ref_file_| **-t** _time_] file ...

## DESCRIPTION

Update the access and modification times of each file to the current time.

A file argument that does not exist is created empty, unless -c is supplied.

### Options

**-a**

: change only the access time

**-c**

: do not create any files

**-m**

: change only the modification time

**-r** *ref_file*

: use this file's times instead of current time

**-t** *time*

: use `[[CC]YY]MMDDhhmm[.SS]` instead of current time
