#!/usr/bin/perl

# fsplice.pl
# This perl script is used as a preprocessor for the web site and
# looks for entries of the form ##filespec## in the incoming source.
# It looks for files matching the spec in the spider's web publishing
# directory, and formats them appropriately as hyperlinks.

while(<>)
{
	if (($filespec) = ($_ =~ /\#\#(.*)\#\#/))
	{
		$line = $_;
		$DIRPIPE = "ls -l ../web/$filespec  |";
		open DIRPIPE or die;
		while(<DIRPIPE>)
		{
			if(($filesize,$filename) = ($_ =~ /(\d+)\s+\S+\s+\d+\s+\d+\:\d+\s+\.\.\/web\/(\S+)/))
			{
				$replace="<a href=\"$filename\">$filename</a> ($filesize bytes)";
			}
		}
		close DIRPIPE;
		$line =~ s/\#\#(.*)\#\#/$replace/;
		print $line;
	}
	else
	{
		print;
	}
}
