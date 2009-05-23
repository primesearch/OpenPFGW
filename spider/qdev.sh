#! /bin/sh

# qdev.sh
#  - All spider scripts begin by performing a cvs update on the entire tree, so this script is current
#  - spider obtains the date tag (e.g. 20020128 for January 28, 2002)
#  - spider tags the entire tree with both DEV and DEV_DATE
#  - spider checks out a separate copy of the dev tree in its 'publish' directory
#  - it prunes the CVS folders from the publish image
#  - it destroys the html source code documentation in the 'publish' directory
#  - it zips up the publish image and places it in web
#  - it zaps the publish image


if ( test "$USER" = "spider")
then
	datestamp=`date +%Y%m%d`
	devtag="DEV"
	devdatetag="DEV-$datestamp"
	devzip="dev-openpfgw-$datestamp.zip"
	
	echo "Moving tag $devtag..."
	(cd .. && cvs -Q rtag -F -a -r HEAD $devtag openpfgw)
	echo "Creating tag $devdatetag..."
	(cd .. && cvs -Q rtag -F -a -r HEAD $devdatetag openpfgw)
	
	echo "Getting a publishable image..."
	(cd ../../publish && cvs -Q co -r $devtag openpfgw)
	
	echo "Pruning CVS folders..."
	(cd ../../publish && ../openpfgw/spider/cvsprune.pl)
	
	echo "Destroying source code documentation..."
	(cd ../../publish/openpfgw && rm html/* && rmdir html)
	(cd ../../publish/openpfgw && rm html_web/* && rmdir html_web)
	
	echo "Removing previous dev drops..."
	(cd ../../web && rm dev*.zip)
	
	echo "Producing dev drop..."
	(cd ../../publish && zip -r -9 -q ../web/$devzip openpfgw)

	echo "Removing the published image..."
	(cd ../../publish && rm -r *)
	
	echo "Stripping unnecessary Web files..."
	(cd ../../web && rm README)
	
else echo "Maybe you should logon first"
fi


