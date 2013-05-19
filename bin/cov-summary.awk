# summarizes the gcov output

BEGIN {
	num = 0;
	sum = 0;
	file_name = "";
	print "";
	print "COVERAGE:";
}

/File.*/ {
	if (!match($2, "lexer.yy.c|parser.tab.c|'lexer.l|parser.y|^'/usr/.*'")) {
		file_name = $2;
		file[file_name] = "0.00%";
		num++;
	}
}

/Lines\ executed:.*/ {
	if (file_name != "") {
		split($2, a, ":");
		file[file_name] = a[2];
		sum += a[2];
		file_name = "";
	}
}

END {
	# sort indices using helper array
	k = 1
	for (f in file) {
		fn[k] = f
		k++
	}
	n = asort(fn)

	# print coverage sorted in ascending filenames
	for (i = 1; i <= n; ++i) {
		printf "%10s : %s\n", file[fn[i]], fn[i];
	}

	print "";
	printf "%9.2f%% AVERAGE (sum:%s, num:%s)\n", sum / num, sum, num;
	print "";
}

