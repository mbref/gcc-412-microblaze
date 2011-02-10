#  Copyright (C) 2003,2004 Free Software Foundation, Inc.
#  Contributed by Kelley Cook, June 2004.
#  Original code from Neil Booth, May 2003.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any
# later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

# This Awk script reads in the option records generated from 
# opt-gather.awk, combines the flags of duplicat options and generates a
# C file.
#
# This program uses functions from opt-functions.awk
#
# Usage: awk -f opt-functions.awk -f optc-gen.awk \
#            [-v header_name=header.h] < inputfile > options.c

BEGIN {
	n_opts = 0
	n_langs = 0
        quote = "\042"
	comma = ","
	FS=SUBSEP
	# Default the name of header created from opth-gen.awk to options.h
	if (header_name == "") header_name="options.h"
}

# Collect the text and flags of each option into an array
	{
		if ($1 == "Language") {
			langs[n_langs] = $2
			n_langs++;
		}
		else {
			name = opt_args("Mask", $1)
			if (name == "") {
				opts[n_opts]  = $1
				flags[n_opts] = $2
				help[n_opts]  = $3
				n_opts++;
			}
		}
	}

# Dump that array of options into a C file.
END {
print "/* This file is auto-generated by opts.sh.  */"
print ""
n_headers = split(header_name, headers, " ")
for (i = 1; i <= n_headers; i++)
	print "#include " quote headers[i] quote
print "#include " quote "opts.h" quote
print "#include " quote "intl.h" quote
print ""
print "int target_flags;"
print ""

for (i = 0; i < n_opts; i++) {
	name = var_name(flags[i]);
	if (name == "")
		continue;

	if (flag_set_p("VarExists", flags[i])) {
		# Need it for the gcc driver.
		if (name in var_seen)
			continue;
		init = ""
	}
	else {
		init = opt_args("Init", flags[i])
		if (init != "")
			init = " = " init;
		else if (name in var_seen)
			continue;
	}

	print "/* Set by -" opts[i] "."
	print "   " help[i] "  */"
	print var_type(flags[i]) name init ";"
	print ""

	var_seen[name] = 1;
}

print ""
print "/* Local state variables.  */"
for (i = 0; i < n_opts; i++) {
	name = static_var(opts[i], flags[i]);
	if (name != "")
		print "static " var_type(flags[i]) name ";"
}
print ""

print "const char * const lang_names[] =\n{"
for (i = 0; i < n_langs; i++) {
	macros[i] = "CL_" langs[i]
	gsub( "[^A-Za-z0-9_]", "X", macros[i] )
	s = substr("         ", length (macros[i]))
	print "  " quote langs[i] quote ","
    }

print "  0\n};\n"
print "const unsigned int cl_options_count = N_OPTS;\n"

print "const struct cl_option cl_options[] =\n{"

j = 0
for (i = 0; i < n_opts; i++) {
	back_chain[i] = "N_OPTS";
	indices[opts[i]] = j;
	# Combine the flags of identical switches.  Switches
	# appear many times if they are handled by many front
	# ends, for example.
	while( i + 1 != n_opts && opts[i] == opts[i + 1] ) {
		flags[i + 1] = flags[i] " " flags[i + 1];
		i++;
		back_chain[i] = "N_OPTS";
		indices[opts[i]] = j;
	}
	j++;
}


for (i = 0; i < n_opts; i++) {
	# Combine the flags of identical switches.  Switches
	# appear many times if they are handled by many front
	# ends, for example.
	while( i + 1 != n_opts && opts[i] == opts[i + 1] ) {
		flags[i + 1] = flags[i] " " flags[i + 1];
		i++;
	}

	len = length (opts[i]);
	enum = "OPT_" opts[i]
	if (opts[i] == "finline-limit=")
		enum = enum "eq"
	gsub ("[^A-Za-z0-9]", "_", enum)

	# If this switch takes joined arguments, back-chain all
	# subsequent switches to it for which it is a prefix.  If
	# a later switch S is a longer prefix of a switch T, T
	# will be back-chained to S in a later iteration of this
	# for() loop, which is what we want.
	if (flag_set_p("Joined.*", flags[i])) {
		for (j = i + 1; j < n_opts; j++) {
			if (substr (opts[j], 1, len) != opts[i])
				break;
			back_chain[j] = enum;
		}
	}

	s = substr("                                  ", length (opts[i]))
	if (i + 1 == n_opts)
		comma = ""

	if (help[i] == "")
		hlp = "0"
	else
		hlp = quote help[i] quote;

	neg = opt_args("Negative", flags[i]);
	if (neg != "")
		idx = indices[neg]
	else {
	if (flag_set_p("RejectNegative", flags[i]))
		idx = -1;
	else {
			if (opts[i] ~ "^[Wfm]")
				idx = indices[opts[i]];
			else
				idx = -1;
		}
	}
	printf("  { %c-%s%c,\n    %s,\n    %s, %u, %d,\n",
	       quote, opts[i], quote, hlp, back_chain[i], len, idx)

	condition = opt_args("Condition", flags[i])
	cl_flags = switch_flags(flags[i])
	if (condition != "")
		printf("#if %s\n" \
		       "    %s,\n" \
		       "#else\n" \
		       "    CL_DISABLED,\n" \
		       "#endif\n",
		       condition, cl_flags, cl_flags)
	else
		printf("    %s,\n", cl_flags)
	printf("    %s, %s }%s\n", var_ref(opts[i], flags[i]),
	       var_set(flags[i]), comma)
}

print "};"
}
