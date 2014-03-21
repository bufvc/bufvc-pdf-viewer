<?php

// This should be OK if you have built gviewer using make
define('GVIEWER', '/usr/local/bin/gviewer');

// You may need to change this for your local system
define("BASE_URL", "http://localhost/bufvc-pdf-viewer/");

// These should not need changing
define("BASEPATH", dirname(dirname(__FILE__)) . "/");
define("MEDIAPATH", BASEPATH . "media/");
