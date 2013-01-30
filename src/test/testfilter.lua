function filter(mout, min)
	syslog(LOG_NOTICE, "executing filter")
	return FILTER_DISCARD
end

print("start lua filter")
