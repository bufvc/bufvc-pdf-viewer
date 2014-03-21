<?php

session_start();

require 'config.php';

// Use gviewer to convert a PDF to a set of images. Returns the list of images.
function gviewer_convert_file($filename_pdf)
    {
    $filename = explode(".", $filename_pdf);
    $pdf_path = MEDIAPATH . "pdf/";
    $full_filename_pdf = MEDIAPATH . "pdf/$filename_pdf";
    $image_path = MEDIAPATH . "image/" . $filename[0];

    if (!file_exists($image_path))
        mkdir($image_path, 0777, true);

    // This line converts the PDF to JPEGs using the gviewer utility on your server.
    exec(GVIEWER . " -o $image_path/{$filename_pdf}_%d.jpg $full_filename_pdf");

    // Get the thumbnails for display
    $files_image = scandir($image_path);
    $image_array = array();
    foreach ($files_image as $fi)
        {
        if ($fi != "." && $fi != "..")
            {
            if (strstr($fi, '_thumb'))
                {
                $relpath_image = explode(BASEPATH, "$image_path/$fi");
                $image_array[] = BASE_URL . $relpath_image[1];
                }
            }
        }
    return $image_array;
    }

function make_thumbnails_html($image_list)
    {
    $html = "<ul id='list-a'>";
    foreach ($image_list as $il)
        {
        if (strstr($il, '_thumb'))
            {
            $image = explode("_thumb", $il);
            $html .= "<li><a href='#' data-src='" . $image[0] . ".jpg'><img src='$il' height='50px' /></a></li>";
            }
        }
    $html .= "</ul>";
    return $html;
    }

$file_pdf = @$_GET["file"];
if ($file_pdf != '')
    {
    $_SESSION['pdf_file'] = $file_pdf;
    if (file_exists(MEDIAPATH . "pdf/$file_pdf"))
        {
        $image_list = gviewer_convert_file($file_pdf);
        $thumbs_html = make_thumbnails_html($image_list);
        $list_image = array();
        foreach ($image_list as $il)
            {
            $image = explode("_thumb", $il);
            $list_image[] = $image[0] . ".jpg";
            }
        $image_list = $list_image;
        $link = $list_image[0];
        }
    }

?>
<!DOCTYPE html>
<html lang="en-GB" xml:lang="en-GB" xmlns="http://www.w3.org/1999/xhtml">
    <head>

        <title>BUFVC PDF Viewer</title>

        <meta charset="UTF-8" />
        <meta http-equiv="imagetoolbar" content="no" />
        <script src="http://code.jquery.com/jquery-1.11.0.min.js"></script>
        <link rel="stylesheet" type="text/css" href="<?php echo BASE_URL; ?>php/js/zoomify/zoomify.css" media="all" />
        <script type="text/javascript" src="<?php echo BASE_URL; ?>php/js/zoomify/zoomify.js"></script>
        <script type="text/javascript" src="<?php echo BASE_URL; ?>php/js/fullscreen.js"></script>
        <style>
            body{background: black}
            h1,p {color: white; }
            ul#list-a { list-style: none; display: block; text-align:center; }
            ul#list-a li{display: inline-block}
            ul#list-a li a { display: block; width: 100px; height: 100px; margin-bottom: 20px; overflow: hidden; }
            ul#list-a li a img { border: none; }
        </style>
    </head>
    <body>
        <h1>BUFVC PDF Viewer</h1>
        <p>Please upload a PDF and it will be viewable in the window below.</p>
        <div id="pdf-wrapper">
            <div class="zoom-full"></div>
            <div id="image-zoom-wrapper">
                <img id="image-zoom" src="<?php echo $link; ?>" alt="No file uploaded, or file not found" />
            </div>
            <div class="download-pdf"><a href="<?php echo BASE_URL; ?>media/pdf/<?php echo $file_pdf; ?>"> <img src="<?php echo BASE_URL; ?>php/js/zoomify/pdficon.png"/><?php echo $file_pdf; ?> </a></div>
            <?php echo $thumbs_html; ?>
        </div>
        </div>
        <script>
            $(document).ready(function() {
                jQuery('#list-a a').on({
                    'click': function() {
                        jQuery('#image-zoom').attr('src', jQuery(this).attr("data-src"));
                    }
                });
                jQuery(document).on('click', '.zoom-full', function() {
                    $(document).fullScreen(true);
                });
            });

        </script>
<div class="upload">
<form action="upload_file.php" method="post" enctype="multipart/form-data">
        <input type="file" name="filepdf" /><br /><input type="submit" value="Upload PDF to server" name="upload_pdf" />
</form>
</div>
    </body>
</html>
