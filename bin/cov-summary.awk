# summarizes the gcov output

BEGIN {
	num = 0;
	sum = 0;
	file_name = "";
	print "";
	print "COVERAGE:";
}

/File.*/ {
	if (!match($2, "'lexer.l|parser.y|^'/usr/.*'")) {
		file_name = $2;
		file[file_name] = 0;
	}
}

/Lines\ executed:.*/ {
	if (file_name != "") {
		split($2, a, ":");
		file[file_name] = a[2];
		num++;
		sum += a[2];
		file_name = "";
	}
}

END {
	for (i in file) {
		printf "%10s %s\n", file[i], i;
	}
	print "";
	printf "%9.2f%% AVERAGE\n", sum / num;
	print "";
}

