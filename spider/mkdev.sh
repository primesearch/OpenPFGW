#! /bin/sh

# mkdev.sh
#  - All spider scripts begin by performing a cvs update on the entire tree, so this script is current
#  - spider obtains the date tag (e.g. 20020128 for January 28, 2002)
#  - spider tags the entire tree with both DEV and DEV_DATE
#  - spider checks out a separate copy of the dev tree in its 'publish' directory
#  - it produces the html source code documentation in the 'publish' directory
#  - it prunes the CVS folders from the publish image
#  - it zips up the html source code documentation
#  - it destroys the html source code documentation in the 'publish' directory
#  - it zips up the publish image and places it in web
#  - it zaps the publish image
#  - it rebuilds the web documentation in the working directory
# - it copies necessary web files from the dox directory into the web directory
#  - it copies the html_web
# - it commits the changes in html_web to the repository


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
	
	echo "Producing HTML source code documentation..."
	(cd ../../publish/openpfgw && doxygen)
	
	echo "Pruning CVS folders..."
	(cd ../../publish && ../openpfgw/spider/cvsprune.pl)
	
	echo "Zipping up source code documentation..."
	(cd ../../publish/openpfgw && zip -r -9 -q ../../web/htmldoc.zip html)
	
	echo "Destroying source code documentation..."
	(cd ../../publish/openpfgw && rm html/* && rmdir html)
	(cd ../../publish/openpfgw && rm html_web/* && rmdir html_web)
	
	echo "Removing previous dev drops..."
	(cd ../../web && rm dev*.zip)
	
	echo "Producing dev drop..."
	(cd ../../publish && zip -r -9 -q ../web/$devzip openpfgw)

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


