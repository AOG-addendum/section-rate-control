#!/usr/bin/python3

Import("env")

env.Replace(PROGNAME="section_rate_71_firmware_%s" % env.GetProjectOption("custom_prog_version"))