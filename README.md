This package allows you to view PDFs online in a browser.

The main part is a command-line utility gviewer which is used to convert a PDF to one or more JPEG images.

A sample PHP script is included so you can use this online and see how to integrate it into your own applications.

This script also allows you to upload a PDF to your server.

PDFs will be converted  to images and displayed in your browser with full functionality, zoom-in, zoom-out,
full-screen, pdf download and (pan) hold and drag.

This system includes XPDF from Foo Labs <http://www.foolabs.com/xpdf/>.

This system is released under the GPL version 2.

Gviewer and the PHP code is copyright (C) 2014 British University Film and Video Council, http://bufvc.ac.uk.
You can contact us by email here: pdf-viewer@bufvc.ac.uk

= How to build gviewer
The instructions below assume an Ubunbtu or Debian variant of Linux, but this has also been built on MacOS and Centos.

Unzip the archive.

Ensure the following Dependences are installed:

    sudo apt-get install libmagickwand-dev imagemagick
    sudo apt-get install libmagick++-dev libmagick++4
    sudo apt-get install libmagickwand-dev
    sudo apt-get install ghostscript

You can then build gviewer with the following commands. You will need root privileges for this:

  cd /[your project directory]/gviewer-code
  make

This will create the file /usr/local/bin/gviewer and you should run this to test it.

Note: At line 90 of gviewer-code/gviewer/gviewer.cc is this line:

  // assuming that gs is located here...
  #define GS "/usr/bin/gs"

Check with `which gs` to ensure this is correct and change it if necessary.

= Using php script
To use the php functionality you have to make the project directory visible to your web server, either by copying
to /var/www or by adding an Alias in your Apache configuration.

Ensure that the directories media/ and media/pdf are writable by the web server.

Check the file php/config.php. The defaults in there should be OK, change them if necessary for your system.

Then navigate through your browser http://localhost/bufvc-pdf-viewer/php

Upload a pdf file and then wait to be redirected to it.

Any questions or comments, please email pdf-viewer@bufvc.ac.uk.

= Using Gviewer
Gviewer accepts the following options:

Usage: gviewer [options] [PATTERN] <PDF-File>

The PATTERN is a regular expression which can be used to highlight phrases within the PDF. BUFVC uses this to highlight
search terms.

Other options are:

  -f <int>          : First page to convert
  -l <int>          : Last page to convert
  -i                : Ignore case search in pattern
  -q                : Quiet, don't print any results nor errors
  -h                : Print usage information
  -o <string>       : Output path and filename of produced pages: <./foo_%d.jpg>
  -thumbs           : Generate thumbnails only
  -help             : Print usage information
  -v                : Print version
  --debug           : Outputs debuging information
