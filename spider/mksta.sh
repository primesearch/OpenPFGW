#! /bin/sh

# mksta.sh
#  - All spider scripts begin by performing a cvs update on the entire tree, so this script is current
#  - spider obtains the date tag (e.g. 20020128 for January 28, 2002)
#  - spider checks out at the DEV level into the 'publish' directory
#  - spider tags the entire tree with both STA and STA_DATE
#  - it prunes the CVS folders from the publish image
#  - it zips up the publish image and places it in web
#  - it zaps the publish image
#  - it rebuilds the web documentation in the working directory
#  - it copies necessary web files from the dox directory into the web directory
#  - it copies the html_web
#  - it commits the changes in html_web to the repository

if ( test "$USER" = "spider")
then
	datestamp=`date +%Y%m%d`
	devtag="DEV"
	statag="STA"
	stadatetag="STA-$datestamp"
	stazip="sta-openpfgw-$datestamp.zip"
	
	echo "Getting the last DEV version..."
	(cd ../../publish && cvs -Q co -r $devtag openpfgw)
	
	echo "Moving tag $statag..."
	(cd ../../publish/openpfgw && cvs -Q rtag -F -a -r $devtag $statag openpfgw)
	echo "Creating tag $stadatetag..."
	(cd ../../publish/openpfgw && cvs -Q rtag -F -a -r $devtag $stadatetag openpfgw)
	
	echo "Pruning CVS folders..."
	(cd ../../publish && ../openpfgw/spider/cvsprune.pl)
	
	echo "Destroying source code documentation..."
	(cd ../../publish/openpfgw && rm html/* && rmdir html)
	(cd ../../publish/openpfgw && rm html_web/* && rmdir html_web)
	
	echo "Removing previous sta drops..."
	(cd ../../web && rm sta*.zip)
	
	echo "Producing sta drop..."
	(cd ../../publish && zip -r -9 -q ../web/$stazip openpfgw)

	echo "Removing the published image..."
	(cd ../../publish && rm -r *)
	
	echo "Rebuilding Web site..."
	(cd .. && doxygen Doxyweb)
	
	echo "Copying Web images..."
	(cd ../dox && cp *.gif ../html_web)
	
	echo "Copying Web site..."
	(cd ../html_web && cp -f * ../../web)
	
	echo "Stripping unnecessary Web files..."
	(cd ../../web && rm README)
		
	echo "Updating Web site image..."
	(cd ../html_web && cvs -Q add *)
	(cd ../html_web && cvs -Q commit -m "Web site updated")
	
else echo "Maybe you should logon first"
fi


