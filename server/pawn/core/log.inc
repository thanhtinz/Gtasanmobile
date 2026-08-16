/**
 * Console and file logging.
 *
 * open.mp already timestamps console output, so Log() only adds a tag. Admin
 * and economy events additionally go to a dated file so they survive a restart.
 */

#if defined _gtasan_log_included
	#endinput
#endif
#define _gtasan_log_included

#define Log(%0) printf("[gm] " %0)

stock LogToFile(const file[], const line[])
{
	new
		year, month, day, hour, minute, second,
		path[64],
		stamp[32];

	getdate(year, month, day);
	gettime(hour, minute, second);

	format(path, sizeof path, "logs/%s-%04d-%02d.log", file, year, month);
	format(stamp, sizeof stamp, "[%02d/%02d %02d:%02d:%02d] ", day, month, hour, minute, second);

	new File:handle = fopen(path, io_append);
	if (!handle)
	{
		return 0;
	}

	fwrite(handle, stamp);
	fwrite(handle, line);
	fwrite(handle, "\r\n");
	fclose(handle);
	return 1;
}

stock LogAdmin(const line[])
{
	printf("[admin] %s", line);
	return LogToFile("admin", line);
}

stock LogEconomy(const line[])
{
	return LogToFile("economy", line);
}
