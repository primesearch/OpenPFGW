#!/usr/bin/perl

#This perl script is designed to parse Chris Caldwell's database of the
#largest known primes (downloadable as all.txt)

#

$fileindex="00";
$fn=">test$fileindex.txt";

open(FN,$fn);

while(<>)
{
# primes match the following pattern
# a space
# one to four digits (with leading spaces)
# optional recent flag
# a space
# prime string
	if( ($rank, $prime) = ($_ =~ /^\s(.{4}?).\s(\S+)\s/) )
	{
		if( ($rank) = ($rank =~ /^\s*(\d+)$/) )
		{
# the prime and the rank are extracted at this point.
			print FN "$prime\n";
			
			if(($rank%100)==0)
			{
				close(FN);
				($fileindex) =("0$rank" =~ /(.{2}?).{2}?$/);
				$fn=">test$fileindex.txt";
				open(FN,$fn);
			}
		}
	}
}

close(FN);
