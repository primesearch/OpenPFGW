#!/usr/bin/perl

# Prune CVS directories from a distribution

$DIRPIPE = "ls -R | grep CVS: |";

open DIRPIPE or die;
while(<DIRPIPE>)
{
	if(($dir) = ($_ =~ /(.*)\:/))
	{
		system "rm -f $dir/*";
		system "rmdir $dir";
	}
}
close DIRPIPE;
