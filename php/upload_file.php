<?php

require 'config.php';

$pdfpath = MEDIAPATH . 'pdf/';

if ($_SERVER['REQUEST_METHOD'] == 'POST' && isset($_POST['upload_pdf']))
    {   
    if (!is_uploaded_file($_FILES['filepdf']['tmp_name']))
        {
        echo "<script language='javascript'>\n";
        echo "alert('No upload file specified'); window.location.href='index.php';";
        echo "</script>\n";
        }
    else {
        $docName = str_replace(' ', '_', basename( $_FILES['filepdf']['name']));
        $result = move_uploaded_file($_FILES['filepdf']['tmp_name'], $pdfpath . $docName);
        if ($result)
            {
            echo '<p class="error">Redirecting to document</p>';
            echo "<script language='javascript'>\n";
            echo "alert('Upload successful!'); window.location.href='index.php?file=$docName';";
            echo "</script>\n";
            }
        else {
            echo '<p class="error">Sorry, there has been an error. Please check the file type or file size.</p>';
            }
        }
    }
