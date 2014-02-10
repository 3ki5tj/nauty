nauty.zip::
	zip -r nauty.zip na* GNUmakefile --exclude=".git" \
	  --exclude="*.tar.gz" --exclude="*.zip" \
	  --exclude="*.o" --exclude="*.a" \
	  --exclude="*.gch" --exclude="*.pch" --exclude="*g" \
	  --exclude="watercluster2" --exclude="dreadnaut" \
	  --exclude="runalltests" \
	  --exclude="*.alw" --exclude="*.ps" --exclude="*.pdf" \
	  --exclude="*~" --exclude=".*.swp"

zip: nauty.zip


usb: zip
	mv nauty.zip /media/C3/code/

