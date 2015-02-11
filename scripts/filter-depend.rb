#!/usr/bin/env ruby
#
# filter-depend.pl
#   by Paul Merrill <pdm@pdm.me>
#
# Read a list of GNU make dependencies from stdin.
# 1) Remove dependencies that use absolute paths. (System-specific deps.)
# 2) Replace basename'd targets with full paths.
# Print to stdout.
#


# Output is wrapped to this many columns.
WIDTH = 78


# Word wrap a string to WIDTH columns. If split over multiple lines, all but
# the last line are suffixed with a backslash and all but the first line are
# indented with a space.
def wrap space, token, rest
	len = token.length
	if space - 2 < len
		"\\\n " + wrap(WIDTH - 1, token, rest)
	elsif rest.empty? == false
		"#{token} " + wrap(space - len - 1, rest.shift, rest)
	else
		"#{token}\n"
	end
end


def main
	# Same files as listed in 'make depend'
	targets = `echo *.cpp */*.cpp`.split.map { |file|
		file.sub /\.cpp$/, '.o'
	}.to_enum

	# The compiler performs a `dirname` on any target it processes. We want the
	# original file paths, though.
	target_filename_remaps = Hash[targets.map { |file|
		[`basename #{file}`.chomp, file]
	}]

	# Get dependencies input from compiler
	raw_lines = $stdin.readlines.map { |line| line.chomp }

	# Join lines that the compiler has wrapped with "\" at end
	lines = []
	while raw_lines.empty? == false
		# Take all lines in a row ending with '\'
		line_parts = raw_lines.take_while { |raw_line|
			raw_line.chomp.end_with? '\\'
		}
		raw_lines.shift line_parts.length

		# Plus one line not ending with '\'
		line_parts << raw_lines.shift

		lines << line_parts.map { |part| part.sub /\\$/, '' }.join
	end

	# Each line is of the form "target: dep1 dep2 dep3..." where target is a
	# compilation target and depX is a dependency.
	#
	# Fix the target's file path and filter out irrelevant dependencies.
	lines.each do |line|
		words = line.split

		target, deps = words.shift, words

		# Restore the path of the target (because it has been stripped to just a
		# basename by the compiler)
		target = target_filename_remaps[target.sub(/:$/, '')] + ':'

		# Clean up dependencies to not include system files.
		deps = deps.find_all { |dep| dep.start_with?('/') == false }

		# Re-wrap to fixed width with "\" at line end as necessary
		puts wrap(WIDTH, target, deps)
	end
end

main
