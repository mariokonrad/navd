# summarizes the gcov output

BEGIN {
	file_name = "";
	print "";
	print "COVERAGE:";
}

/File.*/ {
	file_name = $2;
	file[file_name] = 0;
}

/Lines\ executed:.*/ {
	if (file_name != "") {
		split($2, a, ":");
		file[file_name] = a[2];
		file_name = "";
	}
}

END {
	for (i in file) {
		printf "%10s %s\n", file[i], i;
	}
	print "";
}

